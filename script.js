const $ = (s) => document.querySelector(s);
const $$ = (s) => [...document.querySelectorAll(s)];

const stations = METRO_DATA.stations;
const touristPlaces = METRO_DATA.touristPlaces;
const defaultCards = METRO_DATA.cards;

// The source C++ graph uses consecutive stations on the Red Line,
// with weight 1 for every adjacent station.
const graph = stations.map(() => []);
for (let i = 0; i < stations.length - 1; i++) {
  graph[i].push({to:i+1, weight:1});
  graph[i+1].push({to:i, weight:1});
}

function fillSelect(select, values, placeholder) {
  select.innerHTML = `<option value="">${placeholder}</option>` +
    values.map(v => `<option value="${escapeHtml(v)}">${escapeHtml(v)}</option>`).join("");
}
function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, c => ({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
}

fillSelect($("#source"), stations, "Select source station");
fillSelect($("#destination"), stations, "Select destination");
fillSelect($("#touristSelect"), Object.keys(touristPlaces), "Select a tourist place");

$("#stationCount").textContent = stations.length;
$("#tourCount").textContent = Object.keys(touristPlaces).length;
$("#cardCount").textContent = "∞";

function showView(view) {
  $$(".view").forEach(v => v.classList.toggle("active", v.id === view));
  $$(".nav-item").forEach(n => n.classList.toggle("active", n.dataset.view === view));
  const titles = {dashboard:"Metro Dashboard",route:"Route Planner",tourist:"Tourist Station Finder",card:"Smart Card",map:"Metro Map"};
  $("#pageTitle").textContent = titles[view] || "Metro Dashboard";
  window.scrollTo({top:0, behavior:"smooth"});
}
$$(".nav-item").forEach(btn => btn.addEventListener("click", () => showView(btn.dataset.view)));
$$("[data-go]").forEach(btn => btn.addEventListener("click", () => showView(btn.dataset.go)));

$("#swap").addEventListener("click", () => {
  const a = $("#source").value, b = $("#destination").value;
  $("#source").value = b; $("#destination").value = a;
});

function getPath(src, dest, mode) {
  if (src === dest) return {path:[src], distance:0};
  const n = stations.length;
  const parent = Array(n).fill(-1);

  if (mode === "shortest") {
    const visited = Array(n).fill(false);
    const q = [src]; visited[src] = true;
    while (q.length) {
      const u = q.shift();
      for (const edge of graph[u]) {
        if (!visited[edge.to]) {
          visited[edge.to] = true; parent[edge.to] = u; q.push(edge.to);
        }
      }
    }
  } else {
    const dist = Array(n).fill(Infinity);
    const used = Array(n).fill(false);
    dist[src] = 0;
    for (let k=0;k<n;k++) {
      let u=-1, best=Infinity;
      for(let i=0;i<n;i++) if(!used[i] && dist[i]<best){best=dist[i];u=i;}
      if(u===-1) break;
      used[u]=true;
      for(const edge of graph[u]){
        if(dist[u]+edge.weight<dist[edge.to]){
          dist[edge.to]=dist[u]+edge.weight; parent[edge.to]=u;
        }
      }
    }
  }

  const path=[]; let cur=dest;
  while(cur!==-1){path.unshift(cur); if(cur===src) break; cur=parent[cur];}
  return path[0]===src ? {path,distance:path.length-1} : null;
}

$("#findRoute").addEventListener("click", () => {
  const srcName=$("#source").value, destName=$("#destination").value;
  if(!srcName || !destName) return toast("Please select both stations.");
  const src=stations.indexOf(srcName), dest=stations.indexOf(destName);
  const mode=document.querySelector('input[name="routeType"]:checked').value;
  const result=getPath(src,dest,mode);
  if(!result) return toast("Route not available.");
  const names=result.path.map(i=>stations[i]);
  $("#routeResult").classList.remove("hidden");
  $("#routeResult").innerHTML=`
    <div class="result-head"><h4>${mode==="economic"?"Most Economical Route":"Shortest Route"}</h4><span class="success">● Route Found</span></div>
    <div class="route-line">${names.map((n,i)=>`<span class="station">${escapeHtml(n)}</span>${i<names.length-1?'<span class="arrow">→</span>':''}`).join("")}</div>
    <div class="metrics">
      <div class="metric"><small>Total Stops</small><b>${result.distance}</b></div>
      <div class="metric"><small>Stations Visited</small><b>${names.length}</b></div>
      <div class="metric"><small>Metro Line</small><b>Red Line</b></div>
    </div>`;
});

$("#findTourist").addEventListener("click", () => {
  const place=$("#touristSelect").value;
  if(!place) return toast("Please select a tourist destination.");
  const station=touristPlaces[place];
  const details=METRO_DATA.touristDetails?.[place] || {station,lastMile:"Auto / E-rickshaw",distance:"See map",category:"Tourist Place"};
  $("#touristResult").classList.remove("hidden");
  $("#touristResult").innerHTML=`
    <div class="result-head"><h4>Tourist Connectivity</h4><span class="success">● Found</span></div>
    <div style="margin-top:16px;font-size:15px"><b>${escapeHtml(place)}</b></div>
    <div class="route-line"><span class="station">🚇 ${escapeHtml(details.station)}</span></div>
    <div class="metrics">
      <div class="metric"><small>Category</small><b>${escapeHtml(details.category)}</b></div>
      <div class="metric"><small>Nearest Metro</small><b>${escapeHtml(details.station)}</b></div>
      <div class="metric"><small>Approx. Distance</small><b>${escapeHtml(details.distance)}</b></div>
      <div class="metric"><small>Last Mile</small><b>${escapeHtml(details.lastMile)}</b></div>
    </div>
    <p class="note">Distance and last-mile mode are approximate planning guidance; verify local road conditions before travel.</p>`;
});

function loadBalances(){
  try { return JSON.parse(localStorage.getItem("lucknowMetroBalances")) || {...defaultCards}; }
  catch { return {...defaultCards}; }
}
function saveBalances(b){localStorage.setItem("lucknowMetroBalances",JSON.stringify(b));}

function getCardId(){
  return $("#cardId").value.trim().replace(/\s+/g, "");
}

function updateBalance(){
  const b=loadBalances(), id=getCardId();
  if(!id){
    $("#currentBalance").textContent="₹0";
    return;
  }
  $("#currentBalance").textContent=b[id]!==undefined ? `₹${b[id]}` : "Card not found";
}

$("#cardId").addEventListener("input", updateBalance);
$("#cardId").addEventListener("keydown", (e) => {
  if(e.key === "Enter") $("#recharge").click();
});
updateBalance();

$("#recharge").addEventListener("click", () => {
  const id=getCardId(), amount=Number($("#amount").value);

  if(!id) return toast("Please enter your Smart Card Number.");
  if(!/^\d{6,20}$/.test(id))
    return toast("Enter a valid Smart Card Number (6-20 digits).");
  if(!Number.isFinite(amount)||amount<=0)
    return toast("Enter a valid recharge amount.");

  const b=loadBalances();

  // For this college-project demo, a user-entered card number is accepted
  // and gets a starting balance of ₹0 if it has not been used before.
  // In a production system, this would be replaced by an official Metro API/database.
  if(b[id]===undefined) b[id]=0;

  const before=Number(b[id]), after=before+amount;
  b[id]=after;
  saveBalances(b);
  updateBalance();
  $("#amount").value="";

  $("#cardResult").classList.remove("hidden");
  $("#cardResult").innerHTML=`<div class="result-head"><h4>Recharge Successful</h4><span class="success">● Completed</span></div>
  <div class="metrics" style="margin-top:18px">
    <div class="metric"><small>Card Number</small><b>${escapeHtml(id)}</b></div>
    <div class="metric"><small>Previous Balance</small><b>₹${before}</b></div>
    <div class="metric"><small>Recharge</small><b>₹${amount}</b></div>
    <div class="metric"><small>New Balance</small><b>₹${after}</b></div>
  </div>`;
  toast("Smart card recharged successfully.");
});

function toast(msg){
  const t=$("#toast"); t.textContent=msg; t.classList.add("show");
  setTimeout(()=>t.classList.remove("show"),2500);
}
