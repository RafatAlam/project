// server.js
import fs from "fs";
import path from "path";
import express from "express";
import crypto from "crypto";
import { fileURLToPath } from "url";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const app = express();
app.use(express.json());
app.use(express.static(path.join(__dirname, "public")));

const DATA_DIR = path.join(__dirname, "data");
const USERS_CSV = path.join(DATA_DIR, "users.csv");

fs.mkdirSync(DATA_DIR, { recursive: true });
if (!fs.existsSync(USERS_CSV)) {
  fs.writeFileSync(USERS_CSV, "username,password,avatar\n");
}

// ----------------------
// Sessions & users
// ----------------------

const sessions = new Map(); // token -> username

function readUsers() {
  const text = fs.readFileSync(USERS_CSV, "utf8").trim();
  if (!text) return new Map();
  const lines = text.split("\n").slice(1);
  const map = new Map();
  for (const line of lines) {
    if (!line) continue;
    const [u, p, ...rest] = line.split(",");
    map.set(u, { password: p, avatar: rest.join(",") || "" });
  }
  return map;
}

function getCurrentUser(req) {
  const cookie = req.headers.cookie || "";
  const m = cookie.match(/sid=([a-f0-9]+)/i);
  if (!m) return null;
  const token = m[1];
  return sessions.get(token) || null;
}

// --------------
// Auth routes
// --------------

app.post("/api/signup", (req, res) => {
  const { username, password, avatar = "" } = req.body || {};
  if (!username || !password) {
    return res.status(400).json({ message: "missing fields" });
  }
  const users = readUsers();
  if (users.has(username)) {
    return res.status(409).json({ message: "username taken" });
  }
  fs.appendFileSync(USERS_CSV, `${username},${password},${avatar}\n`);
  res.json({ message: "account created" });
});

app.post("/api/login", (req, res) => {
  const { username, password } = req.body || {};
  if (!username || !password) {
    return res.status(400).json({ message: "missing fields" });
  }
  const users = readUsers();
  const u = users.get(username);
  if (!u || u.password !== password) {
    return res.status(401).json({ message: "invalid credentials" });
  }

  const token = crypto.randomBytes(16).toString("hex");
  sessions.set(token, username);
  res.setHeader("Set-Cookie", `sid=${token}; Path=/; HttpOnly`);
  res.json({ message: "logged in" });
});

app.post("/api/logout", (req, res) => {
  const cookie = req.headers.cookie || "";
  const m = cookie.match(/sid=([a-f0-9]+)/i);
  if (m) sessions.delete(m[1]);
  res.setHeader("Set-Cookie", "sid=; Path=/; Max-Age=0");
  res.json({ message: "logged out" });
});

app.get("/api/me", (req, res) => {
  const user = getCurrentUser(req);
  if (!user) return res.status(401).json({ user: null });
  res.json({ user });
});

// ----------------------
// In-memory rooms + games
// ----------------------

// roomCode -> { players: [username1, username2?], game: GameState }
const rooms = new Map();

/**
 * Game state:
 * {
 *   board: 8x8 array of { type, color, moved }
 *   turn: "WHITE" | "BLACK"
 *   state: "INPROGRESS" | "CHECK" | "CHECKMATE" | "DRAW"
 */

const RANDOM_POOL = [
  "QUEEN",
  "ROOK", "ROOK",
  "BISHOP", "BISHOP",
  "KNIGHT", "KNIGHT",
  "PAWN", "PAWN", "PAWN", "PAWN", "PAWN", "PAWN", "PAWN", "PAWN",
];

function makeEmptyBoard() {
  const board = [];
  for (let r = 0; r < 8; r++) {
    board.push(new Array(8).fill(null));
  }
  return board;
}

function shuffledPool() {
  const arr = [...RANDOM_POOL];
  for (let i = arr.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [arr[i], arr[j]] = [arr[j], arr[i]];
  }
  return arr;
}

// This matches RevealBoard's constructor: kings fixed on e1/e8,
// everything else drawn from a random pool.
function makeNewGame() {
  const board = makeEmptyBoard();

  const whitePieces = shuffledPool();
  const blackPieces = shuffledPool();

  // White side: rows 0 and 1, king at (0,4)
  let count = 0;
  for (let r = 0; r < 2; r++) {
    for (let c = 0; c < 8; c++) {
      if (r === 0 && c === 4) {
        board[r][c] = { type: "KING", color: "WHITE", moved: false };
      } else {
        board[r][c] = {
          type: whitePieces[count++],
          color: "WHITE",
          moved: false,
        };
      }
    }
  }

  // Black side: rows 7 and 6, king at (7,4)
  count = 0;
  for (let r = 7; r >= 6; r--) {
    for (let c = 0; c < 8; c++) {
      if (r === 7 && c === 4) {
        board[r][c] = { type: "KING", color: "BLACK", moved: false };
      } else {
        board[r][c] = {
          type: blackPieces[count++],
          color: "BLACK",
          moved: false,
        };
      }
    }
  }

  return {
    board,
    turn: "WHITE",
    state: "INPROGRESS",
  };
}

function inBounds(r, c) {
  return r >= 0 && r < 8 && c >= 0 && c < 8;
}

// This mirrors Board::getInitialPieceType in C++
function getInitialType(row, col) {
  if (row === 1 || row === 6) {
    return "PAWN";
  } else if (row === 0 || row === 7) {
    switch (col) {
      case 0:
      case 7:
        return "ROOK";
      case 1:
      case 6:
        return "KNIGHT";
      case 2:
      case 5:
        return "BISHOP";
      case 3:
        return "QUEEN";
      default:
        return "KING";
    }
  }
  return null;
}

// This mirrors RevealBoard::getPieceType:
// - if the piece has moved → use its real random type
// - otherwise → pretend it's the standard starting type
function getPieceTypeForRules(game, r, c) {
  const p = game.board[r][c];
  if (!p) return null;
  if (p.moved) return p.type;
  const init = getInitialType(r, c);
  return init || p.type;
}

function exportGameState(game) {
  const pieces = [];
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const p = game.board[r][c];
      if (!p) continue;

      const t = getPieceTypeForRules(game, r, c);
      const hidden = !p.moved && t !== "KING";

      pieces.push({
        r,
        c,
        type: t,
        color: p.color,
        hidden,
      });
    }
  }
  return {
    pieces,
    turn: game.turn,
    state: game.state,
  };
}

// ----------------------
// Helpers for rule-legal moves / check / mate
// ----------------------

function cloneGame(game) {
  return {
    board: game.board.map(row => row.map(p => (p ? { ...p } : null))),
    turn: game.turn,
    state: game.state,
  };
}

function findKing(game, color) {
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const p = game.board[r][c];
      if (!p || p.color !== color) continue;
      const t = getPieceTypeForRules(game, r, c);
      if (t === "KING") return { r, c };
    }
  }
  return null;
}

function isSquareAttacked(game, r, c, byColor) {
  const board = game.board;

  // Pawn attacks
  const pawnDir = byColor === "WHITE" ? 1 : -1; // white pawns move "down" (increasing row)
  const pawnRow = r - pawnDir;
  for (const dc of [-1, 1]) {
    const pc = c + dc;
    if (inBounds(pawnRow, pc)) {
      const p = board[pawnRow][pc];
      if (p && p.color === byColor) {
        const t = getPieceTypeForRules(game, pawnRow, pc);
        if (t === "PAWN") return true;
      }
    }
  }

  // Knight attacks
  const knightDeltas = [
    [2, 1], [1, 2], [-1, 2], [-2, 1],
    [-2, -1], [-1, -2], [1, -2], [2, -1],
  ];
  for (const [dr, dc] of knightDeltas) {
    const rr = r + dr;
    const cc = c + dc;
    if (!inBounds(rr, cc)) continue;
    const p = board[rr][cc];
    if (p && p.color === byColor) {
      const t = getPieceTypeForRules(game, rr, cc);
      if (t === "KNIGHT") return true;
    }
  }

  // King attacks (adjacent squares)
  for (let dr = -1; dr <= 1; dr++) {
    for (let dc = -1; dc <= 1; dc++) {
      if (dr === 0 && dc === 0) continue;
      const rr = r + dr;
      const cc = c + dc;
      if (!inBounds(rr, cc)) continue;
      const p = board[rr][cc];
      if (p && p.color === byColor) {
        const t = getPieceTypeForRules(game, rr, cc);
        if (t === "KING") return true;
      }
    }
  }

  // Sliding pieces: rooks/queens (orthogonal)
  const rookDirs = [
    [1, 0], [-1, 0], [0, 1], [0, -1],
  ];
  for (const [dr, dc] of rookDirs) {
    let rr = r + dr;
    let cc = c + dc;
    while (inBounds(rr, cc)) {
      const p = board[rr][cc];
      if (p) {
        if (p.color === byColor) {
          const t = getPieceTypeForRules(game, rr, cc);
          if (t === "ROOK" || t === "QUEEN") return true;
        }
        break;
      }
      rr += dr;
      cc += dc;
    }
  }

  // Sliding pieces: bishops/queens (diagonal)
  const bishopDirs = [
    [1, 1], [1, -1], [-1, 1], [-1, -1],
  ];
  for (const [dr, dc] of bishopDirs) {
    let rr = r + dr;
    let cc = c + dc;
    while (inBounds(rr, cc)) {
      const p = board[rr][cc];
      if (p) {
        if (p.color === byColor) {
          const t = getPieceTypeForRules(game, rr, cc);
          if (t === "BISHOP" || t === "QUEEN") return true;
        }
        break;
      }
      rr += dr;
      cc += dc;
    }
  }

  return false;
}

function isInCheck(game, color) {
  const kingPos = findKing(game, color);
  if (!kingPos) return false; // should not happen, but be safe
  const enemy = color === "WHITE" ? "BLACK" : "WHITE";
  return isSquareAttacked(game, kingPos.r, kingPos.c, enemy);
}

function makeMoveOnBoard(game, sr, sc, dr, dc) {
  const board = game.board;
  const piece = board[sr][sc];
  board[dr][dc] = { ...piece, moved: true };
  board[sr][sc] = null;
}

// ----------------------
// Move generation (pseudo-legal, then filtered for king safety)
// ----------------------

// forColor is optional; if provided, we only allow moves for that color
function computeValidMoves(game, sr, sc, forColor = null) {
  const board = game.board;
  if (!inBounds(sr, sc)) return [];
  const piece = board[sr][sc];
  if (!piece) return [];

  if (forColor) {
    if (piece.color !== forColor) return [];
  } else {
    if (piece.color !== game.turn) return [];
  }

  const moves = [];
  const color = piece.color;
  const type = getPieceTypeForRules(game, sr, sc);

  const pushIfValid = (r, c) => {
    if (!inBounds(r, c)) return;
    const target = board[r][c];
    if (!target) {
      moves.push({ r, c });
      return;
    }
    if (target.color !== color) {
      moves.push({ r, c });
    }
  };

  const slideDir = (dr, dc) => {
    let r = sr + dr;
    let c = sc + dc;
    while (inBounds(r, c)) {
      const target = board[r][c];
      if (!target) {
        moves.push({ r, c });
      } else {
        if (target.color !== color) moves.push({ r, c });
        break;
      }
      r += dr;
      c += dc;
    }
  };

  switch (type) {
    case "BISHOP":
      slideDir(1, 1);
      slideDir(1, -1);
      slideDir(-1, 1);
      slideDir(-1, -1);
      break;

    case "ROOK":
      slideDir(1, 0);
      slideDir(-1, 0);
      slideDir(0, 1);
      slideDir(0, -1);
      break;

    case "QUEEN":
      slideDir(1, 0);
      slideDir(-1, 0);
      slideDir(0, 1);
      slideDir(0, -1);
      slideDir(1, 1);
      slideDir(1, -1);
      slideDir(-1, 1);
      slideDir(-1, -1);
      break;

    case "KNIGHT": {
      const deltas = [
        [2, 1],
        [1, 2],
        [-1, 2],
        [-2, 1],
        [-2, -1],
        [-1, -2],
        [1, -2],
        [2, -1],
      ];
      for (const [dr, dc] of deltas) {
        pushIfValid(sr + dr, sc + dc);
      }
      break;
    }

    case "KING": {
      const deltas = [
        [1, 0],
        [-1, 0],
        [0, 1],
        [0, -1],
        [1, 1],
        [1, -1],
        [-1, 1],
        [-1, -1],
      ];
      for (const [dr, dc] of deltas) {
        pushIfValid(sr + dr, sc + dc);
      }
      // no castling
      break;
    }

    case "PAWN": {
      const dir = color === "WHITE" ? 1 : -1;
      const startRow = color === "WHITE" ? 1 : 6;

      const oneStepR = sr + dir;
      const twoStepR = sr + 2 * dir;

      // forward moves
      if (inBounds(oneStepR, sc) && !board[oneStepR][sc]) {
        moves.push({ r: oneStepR, c: sc });

        if (
          sr === startRow &&
          inBounds(twoStepR, sc) &&
          !board[twoStepR][sc]
        ) {
          moves.push({ r: twoStepR, c: sc });
        }
      }

      // captures
      const capCols = [sc - 1, sc + 1];
      for (const cc of capCols) {
        if (!inBounds(oneStepR, cc)) continue;
        const target = board[oneStepR][cc];
        if (target && target.color !== color) {
          moves.push({ r: oneStepR, c: cc });
        }
      }
      break;
    }

    default:
      break;
  }

  return moves;
}

// Filtered version that removes moves leaving own king in check
function computeLegalMoves(game, sr, sc, forColor = null) {
  const board = game.board;
  if (!inBounds(sr, sc)) return [];
  const piece = board[sr][sc];
  if (!piece) return [];

  const color = forColor || piece.color;
  const pseudo = computeValidMoves(game, sr, sc, color);
  const legal = [];

  for (const m of pseudo) {
    const testGame = cloneGame(game);
    makeMoveOnBoard(testGame, sr, sc, m.r, m.c);
    if (!isInCheck(testGame, color)) {
      legal.push(m);
    }
  }

  return legal;
}

function hasAnyLegalMove(game, color) {
  for (let r = 0; r < 8; r++) {
    for (let c = 0; c < 8; c++) {
      const p = game.board[r][c];
      if (!p || p.color !== color) continue;
      const legal = computeLegalMoves(game, r, c, color);
      if (legal.length > 0) return true;
    }
  }
  return false;
}

function updateGameStateAfterMove(game) {
  // after the move, game.turn has been flipped to the side that is about to move
  const sideToMove = game.turn;
  const inCheckNow = isInCheck(game, sideToMove);
  const anyLegal = hasAnyLegalMove(game, sideToMove);

  if (inCheckNow && !anyLegal) {
    game.state = "CHECKMATE";
  } else if (!inCheckNow && !anyLegal) {
    game.state = "DRAW"; // stalemate
  } else if (inCheckNow) {
    game.state = "CHECK";
  } else {
    game.state = "INPROGRESS";
  }
}

// ----------------------
// Room helpers + routes
// ----------------------

function generateRoomCode() {
  return crypto.randomBytes(3).toString("hex").toUpperCase();
}

// requireMember = true → user must be in room.players
function getRoom(req, res, requireMember = false) {
  const user = getCurrentUser(req);
  if (!user) {
    res.status(401).json({ message: "not logged in" });
    return null;
  }

  const codeRaw = req.params.code || "";
  const code = codeRaw.toUpperCase();
  const room = rooms.get(code);
  if (!room) {
    res.status(404).json({ message: "room not found" });
    return null;
  }

  if (requireMember && !room.players.includes(user)) {
    res.status(403).json({ message: "not in this room" });
    return null;
  }

  return { room, user, code };
}

// create room, creator is always White (players[0])
app.post("/api/room", (req, res) => {
  const user = getCurrentUser(req);
  if (!user) {
    return res.status(401).json({ message: "not logged in" });
  }

  let code;
  do {
    code = generateRoomCode();
  } while (rooms.has(code));

  rooms.set(code, {
    players: [user],
    game: makeNewGame(),
  });

  res.json({ room: code });
});

// join room (second player = Black)
app.post("/api/room/:code/join", (req, res) => {
  const user = getCurrentUser(req);
  if (!user) {
    return res.status(401).json({ message: "not logged in" });
  }

  const codeRaw = req.params.code || "";
  const code = codeRaw.toUpperCase();
  const room = rooms.get(code);
  if (!room) {
    return res.status(404).json({ message: "room not found" });
  }

  if (!room.players.includes(user)) {
    if (room.players.length >= 2) {
      return res.status(409).json({ message: "room full" });
    }
    room.players.push(user);
  }

  res.json({ room: code, players: room.players });
});

// anyone logged in can see the board
app.get("/api/room/:code/state", (req, res) => {
  const ctx = getRoom(req, res, false);
  if (!ctx) return;
  const { room } = ctx;
  res.json(exportGameState(room.game));
});

// legal moves for a square (now *rule-legal*, king-safe)
app.get("/api/room/:code/moves", (req, res) => {
  const ctx = getRoom(req, res, false);
  if (!ctx) return;
  const { room } = ctx;

  const sr = Number(req.query.sr);
  const sc = Number(req.query.sc);
  if (!Number.isInteger(sr) || !Number.isInteger(sc)) {
    return res.status(400).json({ message: "bad coords" });
  }

  const moves = computeLegalMoves(room.game, sr, sc);
  res.json(moves);
});

// make a move (only room members, and only their own color)
app.post("/api/room/:code/move", (req, res) => {
  const ctx = getRoom(req, res, true);
  if (!ctx) return;
  const { room, user } = ctx;
  const { game } = room;

  // stop moves if game is already finished
  if (game.state === "CHECKMATE" || game.state === "DRAW") {
    return res.json({ ok: false, message: "Game over." });
  }

  const sr = Number(req.query.sr);
  const sc = Number(req.query.sc);
  const dr = Number(req.query.dr);
  const dc = Number(req.query.dc);

  if (
    !Number.isInteger(sr) ||
    !Number.isInteger(sc) ||
    !Number.isInteger(dr) ||
    !Number.isInteger(dc)
  ) {
    return res.status(400).json({ ok: false, message: "bad coords" });
  }

  const index = room.players.indexOf(user); // 0 -> WHITE, 1 -> BLACK
  const myColor = index === 0 ? "WHITE" : "BLACK";

  if (game.turn !== myColor) {
    return res.json({ ok: false, message: "not your turn" });
  }

  // only allow moves that keep own king safe
  const legalMoves = computeLegalMoves(game, sr, sc, game.turn);
  const isLegal = legalMoves.some((m) => m.r === dr && m.c === dc);
  if (!isLegal) {
    return res.json({ ok: false, message: "illegal move" });
  }

  // apply move
  makeMoveOnBoard(game, sr, sc, dr, dc);

  // swap turn, then update check/checkmate/draw state
  game.turn = game.turn === "WHITE" ? "BLACK" : "WHITE";
  updateGameStateAfterMove(game);

  return res.json({ ok: true });
});

// redirect root to login
app.get("/", (req, res) => {
  res.redirect("/login.html");
});

// ----------------------
// Start server
// ----------------------

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
  console.log(`web server → http://localhost:${PORT}`);
});
