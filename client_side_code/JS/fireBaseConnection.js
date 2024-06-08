import { initializeApp } from "https://www.gstatic.com/firebasejs/10.12.2/firebase-app.js";
import {
  getDatabase,
  ref,
  get,
  update,
} from "https://www.gstatic.com/firebasejs/10.12.2/firebase-database.js";

// Your web app's Firebase configuration
const firebaseConfig = {
  apiKey: "AIzaSyDdQZ7k-ZGqIBxL4jkmWG-_44tCmrh3sHU",

  authDomain: "greenhouseproj-da15c.firebaseapp.com",

  databaseURL:
    "https://greenhouseproj-da15c-default-rtdb.europe-west1.firebasedatabase.app",

  projectId: "greenhouseproj-da15c",

  storageBucket: "greenhouseproj-da15c.appspot.com",

  messagingSenderId: "708593777765",

  appId: "1:708593777765:web:33946cfb2f2dbb028dda8c",
};
function showAlert() {
  Swal.fire({
    title: "Success!",
    text: "Pot has been updated successfully",
    icon: "success",
    confirmButtonText: "OK",
  });
}
// Initialize Firebase
const app = initializeApp(firebaseConfig);
const database = getDatabase(app);
// Function to get data
async function getData() {
  const dbRef = ref(database, "device_aa/");
  try {
    const snapshot = await get(dbRef);
    if (snapshot.exists()) {
      console.log(snapshot.val());
      renderData(snapshot.val().pot_data);
    } else {
      console.log("No data available");
    }
  } catch (error) {
    console.error(error);
  }
}
// Function to update data
async function updateData(newData) {
  const dbRef = ref(database, "device_aa/");
  try {
    await update(dbRef, newData);
    showAlert();
  } catch (error) {
    console.error("Update failed: " + error.message);
  }
}

window.addEventListener("load", getData);
setInterval(getData, 5000);

// ---------------------------------------------------------
function renderData(data) {
  let displayData = document.querySelector(".info");
  let oldList = displayData.querySelector("ul");
  if (oldList) {
    displayData.removeChild(oldList);
  }
  let list = document.createElement("ul");
  let cropName = document.createElement("li");
  let temp = document.createElement("li");
  let humidity = document.createElement("li");
  let ledState = document.createElement("li");
  let soilMoi = document.createElement("li");
  let waterLevel = document.createElement("li");
  let doorLevel = document.createElement("li");
  let waterLevelNotif = document.createElement("li");
  cropName.innerHTML = `<span class="key">Crop name</span> : <span class="value">${data.crop}</span`;
  temp.innerHTML = `<span class="key">Temperature</span> : <span class="value">${data.temp} C</span`;
  humidity.innerHTML = `<span class="key">Humidity</span> : <span class="value">${data.humidity}%</span`;
  if (data.led_state === true) {
    ledState.innerHTML = `<span class="key">Led State</span> : <span class="value">Open</span`;
  } else {
    ledState.innerHTML = `<span class="key">Led State</span> : <span class="value">Closed</span`;
  }
  soilMoi.innerHTML = `<span class="key">Soil Mositure</span> : <span class="value">${data.soil_moi}%</span`;
  waterLevel.innerHTML = `<span class="key">Water Level</span> : <span class="value">${data.water_lvl}%</span`;
  // doorLevel.innerHTML = `<span class="key">door Level</span> : <span class="value">${data.door_lvl}</span`;
  if (data.door_lvl == 2) {
    doorLevel.innerHTML = `<div class="alert alert-warning" role="alert">
    <i class="fa-solid fa-triangle-exclamation pr-2"></i>
  Please Close the Pot Cover
</div>`;
  } else if (data.door_lvl == 1) {
    doorLevel.innerHTML = `<div class="alert alert-info" role="alert">
    <i class="fa-solid fa-circle-info pr-2"></i>
  Please Open the Pot Cover
</div>`;
  } else {
    doorLevel.innerHTML = ``;
  }
  if(data.water_lvl < 20)
  {
    waterLevelNotif.innerHTML = `<div class="alert alert-danger" role="alert">
    <i class="fa-solid fa-circle-info pr-2"></i>
      You need to add water to the tank
    </div>`;
  } else {
    waterLevelNotif.innerHTML = ``;
  }
  
  list.append(
    doorLevel,
    waterLevelNotif,
    cropName,
    temp,
    humidity,
    ledState,
    soilMoi,
    waterLevel
  );
  displayData.insertBefore(list, edit);
}

let editBtn = document.querySelector("input[type=checkbox]");
let controlCard = document.querySelector(".controls");

editBtn.addEventListener("click", () => {
  let newPotControl = {};
  if (!editBtn.checked) {
    controlCard.style.display = "block";
    newPotControl.pot_control = {
      auto: false,
      req_soil_moi: Number(soilMoisture.value),
      sprinkle: sprinklerToggle.checked ? true : false,
      sun_led: sunLedToggle.checked ? true : false,
    }
  } else {
    controlCard.style.display = "none";
    newPotControl.pot_control = {
      auto: true,
      req_soil_moi: 0,
      sprinkle: false,
      sun_led: false,
    }
  }
  updateData(newPotControl);
});

// Get references to elements

const soilMoisture = document.getElementById("soilMoisture");
const soilMoistureValue = document.getElementById("soilMoistureValue");
const sprinklerToggle = document.getElementById("sprinklerToggle");
const sunLedToggle = document.getElementById("sunLedToggle");

// Update door level display

// Update soil moisture display
soilMoisture.addEventListener("input", function () {
  soilMoistureValue.textContent = soilMoisture.value;
});
let updateBtn = document.getElementById("update");
updateBtn.addEventListener("click", () => {
  const newPotControl = {
    pot_control: {
      auto: false,
      req_soil_moi: Number(soilMoisture.value),
      sprinkle: sprinklerToggle.checked ? true : false,
      sun_led: sunLedToggle.checked ? true : false,
    },
  };
  updateData(newPotControl);
});
