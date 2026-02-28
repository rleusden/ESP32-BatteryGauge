#include "html.h"

// ------------------- HTML (old Android friendly) -------------------
String getIndexHtml() {
  // Table layout (no grid/flex/vars). XHR polling. No classList.
  return String(R"HTML(
<!doctype html>
<html lang="nl">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
<title>BatteryGauge</title>
<style>
  html,body{height:100%;margin:0;padding:0;}
  body{
    background:#061018;
    color:#e8eef6;
    font-family: Arial, sans-serif;
    overflow:hidden;
  }
  body.dim{
    background:#02070b;
    color:#cfe0f2;
  }

  .wrap{
    height:100%;
    padding:14px;
    box-sizing:border-box;
    max-width:1024px;
    margin:0 auto;
  }

  .top{height:44px;line-height:44px;}
  .brand{float:left;font-weight:bold;font-size:18px;}
  .pill{
    float:right;
    padding:6px 12px;
    border-radius:999px;
    border:1px solid #173246;
    font-size:13px;
    font-weight:bold;
    background:rgba(255,255,255,0.03);
    margin-top:6px;
  }
  .pill.ok{border-color:rgba(53,255,138,0.35); background:rgba(53,255,138,0.10);}
  .pill.warn{border-color:rgba(255,176,32,0.35); background:rgba(255,176,32,0.10);}
  .pill.bad{border-color:rgba(255,70,70,0.35); background:rgba(255,70,70,0.10);}

  .card{
    border:1px solid #173246;
    border-radius:16px;
    background:#0b1620;
    padding:14px;
    box-sizing:border-box;
    height: calc(100% - 44px - 12px);
    position:relative;
  }
  body.dim .card{ background:#061018; }

  .socBig{
    font-size:64px;
    font-weight:bold;
    letter-spacing:-1px;
    display:inline-block;
  }
  .rightInfo{
    float:right;
    text-align:right;
    font-size:14px;
    color:#9fb2c6;
    font-weight:bold;
    margin-top:10px;
  }
  body.dim .rightInfo{ color:#7f95ab; }

  .bar{
    margin-top:10px;
    height:18px;
    border-radius:999px;
    background:#02080d;
    border:1px solid #1b3142;
    overflow:hidden;
  }
  .fill{
    height:100%;
    width:0%;
    background:#35ff8a; /* default green */
  }

  table{
    width:100%;
    border-collapse:collapse;
    table-layout:fixed;
    margin-top:16px;
  }
  th{
    color:#9fb2c6;
    font-size:14px;
    text-align:center;
    padding:6px 0;
    white-space:nowrap;
  }
  body.dim th{ color:#7f95ab; }

  td{
    font-size:34px;
    font-weight:bold;
    text-align:center;
    padding:10px 0;
    white-space:nowrap;
    font-variant-numeric: tabular-nums;
  }

  .modeBig{
    font-size:34px;
    font-weight:bold;
    text-align:center;
    padding:10px 0;
    white-space:nowrap;
    font-variant-numeric: tabular-nums;
  }

  .ctrls{
    position:absolute;
    left:14px;
    right:14px;
    bottom:14px;
  }
  .btn{
    border:1px solid #173246;
    background:rgba(255,255,255,0.03);
    color:inherit;
    padding:10px 12px;
    border-radius:12px;
    font-size:14px;
    font-weight:bold;
  }
  .small{
    color:#9fb2c6;
    font-size:13px;
    font-weight:bold;
  }
  body.dim .small{ color:#7f95ab; }
  .mono{ font-variant-numeric: tabular-nums; }
</style>
</head>
<body>
  <div class="wrap">
    <div class="top">
      <div class="brand">BatteryGauge</div>
      <div id="statusPill" class="pill bad">NO DATA</div>
      <div style="clear:both;"></div>
    </div>

    <div class="card">
      <div>
        <div class="socBig"><span id="soc">--</span>%</div>
        <div class="rightInfo">
          <div><span id="mode">--</span></div>
          <div class="small mono">age: <span id="age">--</span> ms</div>
        </div>
        <div style="clear:both;"></div>

        <div class="bar"><div id="socFill" class="fill"></div></div>

        <table>
          <tr>
            <th>Pack voltage</th>
            <th>Current</th>
            <th>Temperature</th>
          </tr>
          <tr>
            <td><span id="vpack">--</span>V</td>
            <td><span id="cur">--</span>A</td>
            <td><span id="temp">--</span>C</td>
          </tr>
        </table>

        <table>
          <tr><th>Status</th></tr>
          <tr><td class="modeBig"><span id="modeBig">--</span></td></tr>
        </table>
      </div>

      <div class="ctrls">
        <div style="float:left;" class="small">IP: <span class="mono">192.168.4.1</span></div>
        <div style="float:right;">
          <button class="btn" onclick="toggleDim()">DIM</button>
          <button class="btn" onclick="forceRefresh()">REFRESH</button>
        </div>
        <div style="clear:both;"></div>
      </div>
    </div>
  </div>

<script>
  var lastOkMs = 0;

  function fmt(n, d){
    var x = Number(n);
    if(isNaN(x)) return '--';
    return x.toFixed(d);
  }

  function setPill(state, text){
    var pill = document.getElementById('statusPill');
    pill.className = 'pill ' + state;
    pill.innerHTML = text;
  }

  function blankAll(){
    document.getElementById('soc').innerHTML = '--';
    document.getElementById('vpack').innerHTML = '--';
    document.getElementById('cur').innerHTML = '--';
    document.getElementById('temp').innerHTML = '--';
    document.getElementById('mode').innerHTML = '--';
    document.getElementById('modeBig').innerHTML = '--';
    document.getElementById('socFill').style.width = '0%';
  }

  function applyStale(){
    var now = (new Date()).getTime();
    var age = lastOkMs ? (now - lastOkMs) : 999999;
    document.getElementById('age').innerHTML = String(age);

    if(age < 1500){
      setPill('ok','OK');
    } else if(age < 4000){
      setPill('warn','STALE');
    } else {
      setPill('bad','NO DATA');
      if(age > 60000){ // > 1 minute
        blankAll();
      }
    }
  }

  function setBarColor(isCharging){
    // Green when charging, red when discharging
    var fill = document.getElementById('socFill');
    fill.style.background = isCharging ? '#35ff8a' : '#ff4646';
  }

  function tick(){
    var xhr = new XMLHttpRequest();
    xhr.open('GET', '/json?_=' + (new Date().getTime()), true);
    xhr.onreadystatechange = function(){
      if(xhr.readyState !== 4) return;

      if(xhr.status !== 200){
        applyStale();
        return;
      }

      try{
        var j = JSON.parse(xhr.responseText);

        document.getElementById('soc').innerHTML = fmt(j.soc, 0);
        var w = Math.max(0, Math.min(100, Number(j.soc)));
        document.getElementById('socFill').style.width = w + '%';

        document.getElementById('vpack').innerHTML = fmt(j.vpack, 2);
        document.getElementById('cur').innerHTML   = fmt(j.current, 2);
        document.getElementById('temp').innerHTML  = fmt(j.tempC, 1);

        var m = j.charging ? 'Charging' : 'Discharging';
        document.getElementById('mode').innerHTML = m;
        document.getElementById('modeBig').innerHTML = m;

        setBarColor(!!j.charging);

        lastOkMs = (new Date()).getTime();

        if(j.alarms && ( (j.alarms & 0x0001) || (j.alarms & 0x0002) )){
          setPill('bad','ALARM');
        } else if(j.alarms){
          setPill('warn','WARN');
        } else {
          applyStale();
        }
      } catch(e){
        applyStale();
      }
    };
    xhr.send();
  }

  function toggleDim(){
    if(document.body.className === 'dim') document.body.className = '';
    else document.body.className = 'dim';
  }

  function forceRefresh(){
    lastOkMs = 0;
    tick();
  }

  tick();
  setInterval(tick, 500);
  setInterval(applyStale, 250);
</script>
</body>
</html>
)HTML");
}