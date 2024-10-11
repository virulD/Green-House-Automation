 function toggleWindow(windowId) {
    var status = document.getElementById('status' + windowId.slice(-1));
    var checkbox = document.querySelector(`#${windowId} .switch input`);

    if (checkbox.checked) {
        status.textContent = "On";
    } else {
        status.textContent = "Off";
    }
}

function fetchData(endpoint, elementId) {
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
            document.getElementById(elementId).textContent = this.responseText;
        }
    };
    xhttp.open("GET", endpoint, true);
    xhttp.send();
}

// Call fetchData for each sensor value every second
setInterval(function() {
    fetchData("/ldr", "ldrValue");
    fetchData("/temperature", "temperatureValue");
    fetchData("/soilmoisture", "soilMoistureValue");
}, 1000);

let isManualMode = false;

function toggleMode(isManual) {
    isManualMode = isManual;
    document.getElementById("modeStatus").textContent = isManual ? 'Manual' : 'Automatic';

    // Enable or disable the control buttons based on the mode
    document.getElementById("fanSwitch").disabled = !isManual;
    document.getElementById("lightSwitch").disabled = !isManual;
}

// Update the toggleLed function to work only if isManualMode is true
function toggleLed(led, state) {
    if (isManualMode) {
        const url = `/${led}/${state ? 'on' : 'off'}`;
        fetch(url)
            .then(response => response.text())
            .then(data => {
                console.log(data);
                const statusElement = led === 'ledTemp' ? document.getElementById('fanStatus') : document.getElementById('lightStatus');
                statusElement.textContent = state ? 'On' : 'Off';
            })
            .catch(error => console.error('Error:', error));
    }
}

function updateSensorValues() {
  fetch('/sensorData')
    .then(response => response.json())
    .then(data => {
      document.getElementById('lightValue').textContent = data.light + ' lx';
      document.getElementById('temperatureValue').textContent = data.temperature + ' °C';
      document.getElementById('soilMoistureValue').textContent = data.soilMoisture + ' %';
    })
    .catch(error => {
      console.error('Error:', error);
      document.getElementById('lightValue').textContent = 'Error';
      document.getElementById('temperatureValue').textContent = 'Error';
      document.getElementById('soilMoistureValue').textContent = 'Error';
    });
}



setInterval (function () {
        var xhttp = new XMLHttpRequest();
        xhttp.onreadystatechange = function () {
            if (this.readyState == 4 && this.status == 200) {
                document.getElementById("temperature").innerHTML = this.responseText;
            }
        };
        xhttp.open("GET", "/temperature", true);
        xhttp.send();
}, 1000);    



