// src/web_gui.cpp
// Minimal click-to-move Web GUI (no external libs) using ROW,COL with your Board.
// UI coordinates = (row, col). Engine calls = (row, col) to match your terminal.

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>
#include <execinfo.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

#include "../header/board.hpp"
#include "../header/piece.hpp"
#include "../header/revealBoard.hpp"
#include "../header/game.hpp"

// -------- crash handler (helps if something goes wrong) --------
static void segv_handler(int sig){
  void* bt[32];
  int n = backtrace(bt, 32);
  fprintf(stderr, "\n*** SIGSEGV backtrace:\n");
  backtrace_symbols_fd(bt, n, STDERR_FILENO);
  fprintf(stderr, "*** end ***\n");
  _exit(128 + sig);
}

// ---------------- HTML/JS (row 0 is TOP) ----------------
static const char* kIndexHtml = R"HTML(<!doctype html>
<html lang="en">
<meta charset="utf-8"/>
<title>Chess (Web GUI)</title>
<style>
  html,body{margin:0;height:100%;background:#202124;color:#e8eaed;font-family:system-ui,Segoe UI,Arial}
  .wrap{display:flex;align-items:center;justify-content:center;height:100%}
  canvas{background:#333;border:8px solid #111;box-shadow:0 10px 40px rgba(0,0,0,.35);border-radius:6px;cursor:pointer}
  .hud{position:fixed;left:14px;opacity:.85}
  #top{top:10px}
  #bottom{bottom:10px}
  #turn{top:10px;right:14px;left:auto;text-align:right}
</style>
<div class="wrap">
  <canvas id="board" width="768" height="768"></canvas>
</div>
<div id="top" class="hud">Click a piece, then click a <b>green</b> square to move. Click same square to cancel.</div>
<div id="bottom" class="hud">&nbsp;</div>
<div id="turn" class="hud">Turn:&nbsp;</div>
<script>
const N=8, TILE=96;
const light='#f0d9b5', dark='#b58863';
const cvs=document.getElementById('board'), ctx=cvs.getContext('2d');
const statusEl=document.getElementById('bottom');
const turnEl=document.getElementById('turn');

let pieces=[];           // {r,c,type,color,hidden}
let legal=[];            // [{r,c}]
let selected=null;       // {sr,sc,type,color,hidden}
let hoverSq=null;
let currentTurn='WHITE';
let currentState='INPROGRESS';

// Unicode chess symbols per color/type
const glyph = {
  WHITE: {
    KING:'\u2654', QUEEN:'\u2655', ROOK:'\u2656',
    BISHOP:'\u2657', KNIGHT:'\u2658', PAWN:'\u2659'
  },
  BLACK: {
    KING:'\u265A', QUEEN:'\u265B', ROOK:'\u265C',
    BISHOP:'\u265D', KNIGHT:'\u265E', PAWN:'\u265F'
  }
};

const setStatus=t=>statusEl.textContent=t||'\u00A0';
const setTurn=t=>turnEl.textContent=t||'Turn:\u00A0';

function drawBoard(){
  for(let r=0;r<N;r++)for(let c=0;c<N;c++){
    ctx.fillStyle=((r+c)&1)?dark:light;
    const x=c*TILE, y=r*TILE;     // row 0 is at the TOP
    ctx.fillRect(x,y,TILE,TILE);
  }
}

function centerOf(r,c){ return {x:c*TILE+TILE/2,y:r*TILE+TILE/2}; }

function pixelToSquare(px,py){
  const c=Math.floor(px/TILE), r=Math.floor(py/TILE);
  if(r<0||r>=8||c<0||c>=8) return null;
  return {r,c};
}

// filled = true -> filled text + outline
// filled = false -> outline only (no fill)
function drawGlyph(x,y,ch,color,alpha=1,filled=true){
  ctx.save();
  ctx.globalAlpha=alpha;

  // color is already the visual color string "WHITE" or "BLACK"
  ctx.fillStyle=(color==='WHITE')?'#fff':'#111';
  ctx.strokeStyle=(color==='WHITE')?'#222':'#eee';

  ctx.lineWidth=3;
  ctx.font='bold 56px system-ui,Segoe UI,Arial';
  ctx.textAlign='center';
  ctx.textBaseline='middle';
  if (filled) {
    ctx.fillText(ch,x,y);
  }
  ctx.strokeText(ch,x,y);
  ctx.restore();
}

// Backend -> visual color: swap WHITE/BLACK so white-backend appears at top as black, etc.
function visualColor(p){
  return (p.color === 'WHITE') ? 'BLACK' : 'WHITE';
}

// Helper to get piece glyph for a given visual color + type
function pieceCharFor(color,type){
  const table = glyph[color] || glyph.WHITE;
  return table[type] || '?';
}

function strokeSquare(r,c,style='rgba(50,200,90,0.95)',lw=4){
  const x=c*TILE, y=r*TILE;
  ctx.strokeStyle=style;
  ctx.lineWidth=lw;
  ctx.strokeRect(x+3,y+3,TILE-6,TILE-6);
}

function fillDot(r,c,color='rgba(50,200,90,0.7)'){
  const x=c*TILE+TILE/2, y=r*TILE+TILE/2;
  ctx.fillStyle=color;
  ctx.beginPath();
  ctx.arc(x,y,10,0,Math.PI*2);
  ctx.fill();
}

// Not used by drawPiece any more but kept for completeness if you need it elsewhere
function pieceChar(p){
  const vColor = visualColor(p);
  return pieceCharFor(vColor, p.type);
}

function drawPiece(p){
  const ctr = centerOf(p.r,p.c);
  const vPieceColor   = visualColor(p);                      // visual color for the actual piece
  const vQuestionColor = (vPieceColor === 'WHITE') ?         // question mark is opposite of piece color
                         'BLACK' : 'WHITE';

  if(p.hidden && p.type !== 'KING'){
    // Hidden non-king pieces: question marks with SWAPPED color relative to the piece
    drawGlyph(ctr.x, ctr.y, '?', vQuestionColor, 1, true);
  }else{
    // Real pieces (including kings): outline-only, in vPieceColor
    const ch = pieceCharFor(vPieceColor, p.type);
    drawGlyph(ctr.x, ctr.y, ch, vPieceColor, 1, false);
  }
}

function redraw(){
  drawBoard();
  if(!selected && hoverSq){
    strokeSquare(hoverSq.r,hoverSq.c,'rgba(255,255,255,0.5)',3);
  }
  for(const p of pieces){
    drawPiece(p);
  }
  if(selected){
    strokeSquare(selected.sr,selected.sc,'rgba(90,150,255,0.95)',4);
  }
  if(selected && legal.length){
    for(const m of legal){
      strokeSquare(m.r,m.c);
      fillDot(m.r,m.c);
    }
  }
}

async function GET_json(url){
  const r=await fetch(url);
  return r.json();
}

function updateTurnHud(){
  const nice = (currentTurn === 'WHITE') ? 'White' : 'Black';
  setTurn(`Turn: ${nice}`);
}

async function loadState(){
  const data = await GET_json('/state');   // {pieces:[...], turn:"WHITE/BLACK", state:"..."}
  if (Array.isArray(data)) {
    // fallback (shouldn't happen now, but just in case)
    pieces = data;
  } else {
    pieces = data.pieces || [];
    if (data.turn) currentTurn = data.turn;
    if (data.state) currentState = data.state;
  }
  updateTurnHud();
  redraw();
}

async function loadMoves(sr,sc){
  legal=await GET_json(`/moves?sr=${sr}&sc=${sc}`);
  redraw();
}

async function postMove(sr,sc,dr,dc){
  const r=await fetch(`/move?sr=${sr}&sc=${sc}&dr=${dr}&dc=${dc}`,{method:'POST'});
  const j=await r.json();
  setStatus(j.ok ? `Moved: (${sr},${sc}) → (${dr},${dc})` :
                   `Illegal: (${sr},${sc}) → (${dr},${dc})`);
  return j;
}

function pieceAt(r,c){
  return pieces.find(q=>q.r===r && q.c===c) || null;
}

function isLegalTarget(r,c){
  return legal.some(m=>m.r===r && m.c===c);
}

cvs.addEventListener('mousemove', ev=>{
  const bb=cvs.getBoundingClientRect();
  hoverSq = pixelToSquare(ev.clientX-bb.left, ev.clientY-bb.top);
  redraw();
});

cvs.addEventListener('click', async ev=>{
  const bb=cvs.getBoundingClientRect();
  const sq=pixelToSquare(ev.clientX-bb.left, ev.clientY-bb.top);
  if(!sq) return;

  if(!selected){
    const p = pieceAt(sq.r, sq.c);
    if(p){
      selected={sr:p.r,sc:p.c,type:p.type,color:p.color,hidden:p.hidden};
      setStatus(`Selected ${p.type} at (${p.r},${p.c})`);
      await loadMoves(p.r,p.c);
    }else{
      setStatus('');
      legal=[];
      redraw();
    }
    return;
  }

  if(sq.r===selected.sr && sq.c===selected.sc){
    selected=null;
    legal=[];
    setStatus('');
    redraw();
    return;
  }

  if(isLegalTarget(sq.r,sq.c)){
    try{
      await postMove(selected.sr,selected.sc,sq.r,sq.c);
    }catch(e){
      setStatus('POST /move failed');
    }
    await loadState();      // also refreshes turn indicator
    selected=null;
    legal=[];
    redraw();
    return;
  }

  const p2=pieceAt(sq.r,sq.c);
  if(p2){
    selected={sr:p2.r,sc:p2.c,type:p2.type,color:p2.color,hidden:p2.hidden};
    setStatus(`Selected ${p2.type} at (${p2.r},${p2.c})`);
    await loadMoves(p2.r,p2.c);
    return;
  }

  selected=null;
  legal=[];
  setStatus('');
  redraw();
});

window.addEventListener('load', loadState);
</script>
</html>)HTML";

// ---------- tiny helpers ----------
static std::string http_ok(const char* mime){
  std::ostringstream o;
  o<<"HTTP/1.1 200 OK\r\nContent-Type: "<<mime<<"\r\nConnection: close\r\n\r\n";
  return o.str();
}
static std::string http_bad(){
  return "HTTP/1.1 400 Bad Request\r\nConnection: close\r\n\r\n";
}
static std::string url_decode(const std::string& s){
  std::string o;
  o.reserve(s.size());
  for (size_t i=0;i<s.size();++i){
    if (s[i]=='%' && i+2<s.size()){
      int v=strtol(s.substr(i+1,2).c_str(),nullptr,16);
      o.push_back(char(v));
      i+=2;
    } else if (s[i]=='+') o.push_back(' ');
    else o.push_back(s[i]);
  }
  return o;
}
static bool safe_send(int fd, const std::string& x){
  ssize_t left = (ssize_t)x.size();
  const char* p = x.c_str();
  while (left > 0) {
    ssize_t n = send(fd, p, left, 0);
    if (n < 0) return false;
    left -= n;
    p += n;
  }
  return true;
}
static int qparam_int(const std::string& q, const std::string& key){
  size_t p=q.find(key+"=");
  if(p==std::string::npos) return -999;
  size_t e=q.find('&',p);
  std::string v=(e==std::string::npos)?
    q.substr(p+key.size()+1):
    q.substr(p+key.size()+1,e-(p+key.size()+1));
  return std::atoi(url_decode(v).c_str());
}

int main(){
  signal(SIGPIPE, SIG_IGN);
  struct sigaction sa{};
  sa.sa_handler = segv_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESETHAND;
  sigaction(SIGSEGV, &sa, nullptr);

  RevealBoard board;
  Game game(&board);

  // socket setup
  int s=socket(AF_INET,SOCK_STREAM,0);
  if(s<0){perror("socket"); return 1;}
  int opt=1;
  setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
  sockaddr_in addr{};
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=htonl(INADDR_ANY);
  addr.sin_port=htons(8080);
  if(bind(s,(sockaddr*)&addr,sizeof(addr))<0){perror("bind"); return 1;}
  if(listen(s,16)<0){perror("listen"); return 1;}
  fprintf(stderr,
          "Web GUI on http://localhost:8080  (ssh -L 8080:localhost:8080 <you>@<host>)\n");

  for(;;){
    int c=accept(s,nullptr,nullptr);
    if(c<0){perror("accept"); continue;}

    char buf[32768];
    ssize_t n=recv(c,buf,sizeof(buf)-1,0);
    if(n<=0){ close(c); continue; }
    buf[n]=0;
    std::string req(buf);

    // parse request line safely
    size_t sp1=req.find(' ');
    size_t sp2=(sp1==std::string::npos)?
      std::string::npos:req.find(' ',sp1+1);
    if(sp1==std::string::npos||sp2==std::string::npos){
      safe_send(c, http_bad()+"Bad request\n");
      close(c);
      continue;
    }
    std::string method=req.substr(0,sp1);
    std::string full=req.substr(sp1+1,sp2-sp1-1), path=full, query;
    if(auto qpos=full.find('?'); qpos!=std::string::npos){
      path=full.substr(0,qpos);
      query=full.substr(qpos+1);
    }

    if(method=="GET" && path=="/"){
      safe_send(c, http_ok("text/html; charset=utf-8")+kIndexHtml);
    }
    else if(method=="GET" && path=="/state"){
      std::ostringstream js;
      js<<"{\"pieces\":[";
      bool first=true;
      for(int r=0;r<8;++r){
        for(int c2=0;c2<8;++c2){
          if(!board.isOccupied(r,c2)) continue;
          if(!first) js<<","; first=false;

          PieceType t=board.getPieceType(r,c2);
          PieceColor col=board.getColor(r,c2);
          bool hidden = (!board.pieceMoved(r,c2) && t != KING);

          const char* tn =
            (t==PAWN  ? "PAWN"  :
             t==KNIGHT? "KNIGHT":
             t==BISHOP? "BISHOP":
             t==ROOK  ? "ROOK"  :
             t==QUEEN ? "QUEEN" : "KING");
          const char* cn = (col==WHITE? "WHITE":"BLACK");

          js<<"{\"r\":"<<r
            <<",\"c\":"<<c2
            <<",\"type\":\""<<tn<<"\""
            <<",\"color\":\""<<cn<<"\""
            <<",\"hidden\":"<<(hidden?"true":"false")
            <<"}";
        }
      }
      // add current turn + game state
      PieceColor turn = game.getCurrentTurn();
      GameState gs = game.getGameState();
      const char* turnStr = (turn==WHITE? "WHITE":"BLACK");
      const char* stateStr =
        (gs==INPROGRESS? "INPROGRESS" :
         gs==CHECK     ? "CHECK"      :
         gs==CHECKMATE ? "CHECKMATE"  :
         gs==DRAW      ? "DRAW"       : "UNKNOWN");

      js<<"],\"turn\":\""<<turnStr<<"\",\"state\":\""<<stateStr<<"\"}";
      safe_send(c, http_ok("application/json")+js.str());
    }
    else if(method=="GET" && path=="/moves"){
      int sr=qparam_int(query,"sr"), sc=qparam_int(query,"sc");
      fprintf(stderr, "[/moves] row,col = %d,%d\n", sr, sc);

      std::ostringstream js;
      js<<"[";
      if(sr>=0&&sr<8&&sc>=0&&sc<8){
        if(!board.isOccupied(sr,sc)){
          fprintf(stderr, "  not occupied at (%d,%d)\n", sr, sc);
        }else{
          PieceType t=board.getPieceType(sr,sc);
          PieceColor col=board.getColor(sr,sc);
          fprintf(stderr, "  piece type=%d color=%d\n", (int)t, (int)col);

          auto moves = board.validMoves(sr,sc); // vector<Position> with x=row, y=col
          fprintf(stderr, "  validMoves returned %zu\n", moves.size());
          bool firstM=true;
          for(const auto& m : moves){
            int mr = m.x;   // row
            int mc = m.y;   // col
            if(!firstM) js<<","; firstM=false;
            js<<"{\"r\":"<<mr<<",\"c\":"<<mc<<"}";
          }
        }
      }
      js<<"]";
      safe_send(c, http_ok("application/json")+js.str());
    }
    else if(method=="POST" && path=="/move"){
      int sr=qparam_int(query,"sr"), sc=qparam_int(query,"sc");
      int dr=qparam_int(query,"dr"), dc=qparam_int(query,"dc");
      fprintf(stderr, "[/move] (%d,%d) -> (%d,%d)\n", sr,sc,dr,dc);

      bool ok=false;
      if(sr>=0&&sr<8&&sc>=0&&sc<8&&dr>=0&&dr<8&&dc>=0&&dc<8){
        ok = game.makeMove(sr,sc,dr,dc);
        if (ok)
          fprintf(stderr, "  -> ok\n");
        else
          fprintf(stderr, "  -> rejected (wrong side or illegal)\n");
      } else {
        fprintf(stderr, "  -> rejected (invalid coords)\n");
      }
      safe_send(c, http_ok("application/json")+
                   std::string("{\"ok\":")+(ok?"true":"false")+"}");
    }
    else{
      safe_send(c,
        "HTTP/1.1 404 Not Found\r\nConnection: close\r\n\r\nNot Found");
    }

    close(c);
  }

  return 0;
}
