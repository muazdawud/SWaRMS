var date, time, staNumber, locationName, ambTemp, ambHum;
var staStatus, maxWeight, weight, inTemp, inHum, fanStatus;

function safeSetText(id, text) {
  if(text){
    const el = document.getElementById(id);
    if (el) el.textContent = text;
  }
}

const page = document.getElementById("page");
const navPerf = performance.getEntriesByType("navigation")[0];
const isPageRefresh = navPerf && navPerf.type === "reload";

if (isPageRefresh) {
  sessionStorage.removeItem("dashboard_shifted");
} else if (sessionStorage.getItem("dashboard_shifted") === "true") {
  if (page) page.classList.add("shifted");
}

document.querySelectorAll('#but input[type="submit"]').forEach((button) => {
  button.addEventListener("click", () => {
    if (page) page.classList.add("shifted");
    sessionStorage.setItem("dashboard_shifted", "true");
  });
});

var socket = new WebSocket("ws://" + window.location.hostname + ":81", ["arduino"]);

socket.onopen = function () {
  socket.send("Connect " + new Date());
};

socket.onerror = function (error) {
  console.log("WebSocket Error ", error);
};

socket.onmessage = function (event) {
  const data = event.data;

  if (typeof data === "string" && data.startsWith("Station Info:")) {
    const cleanData = data.replace("Station Info:", "").trim();
    const tokens = cleanData.split(",");

    tokens.forEach((token, index) => {
      switch (index) {
        case 0:
          date = token;
          safeSetText("date", date);
          break;
        case 1:
          time = token;
          safeSetText("time", time);
          break;
        case 2:
          staNumber = token;
          safeSetText("stationNumber", staNumber);
          break;
        case 3:
          locationName = token;
          safeSetText("stationLocation", locationName);
          break;
        case 4:
          ambTemp = token;
          safeSetText("ambientTemperature", ambTemp);
          break;
        case 5:
          ambHum = token;
          safeSetText("ambientHumidity", ambHum);
          break;
        case 6:
          staStatus = token === "1" ? "Filled" : "Not Filled";
          safeSetText("stationStatus", staStatus);
          break;
        case 7:
          maxWeight = parseFloat(token) || 0;
          break;
        case 8:
          const currentVal = parseFloat(token) || 0;
          weight = maxWeight > 0 ? ((maxWeight - currentVal) / maxWeight) * 100 : 0;
          safeSetText("stationWeight", weight.toFixed(2) + "%");
          break;
        case 9:
          inTemp = token;
          safeSetText("innerTemperature", inTemp);
          break;
        case 10:
          inHum = token;
          safeSetText("innerHumidity", inHum);
          break;
        case 11:
          fanStatus = token === "1" ? "Active" : "Not Active";
          safeSetText("stationFan", fanStatus);
          break;
      }
    });
  }
};

socket.onclose = function () {
  console.log("WebSocket socket closed");
};

function refreshPage() {
  console.log("Fetching station info...");
  if (socket.readyState === WebSocket.OPEN) {
    socket.send("Refresh");
  } else {
    console.warn("WebSocket is not open. Current state:", socket.readyState);
  }
}