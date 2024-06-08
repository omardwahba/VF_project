function showAlert() {
  Swal.fire({
    title: "Success!",
    text: "",
    icon: "success",
    confirmButtonText: "OK",
  });
}

let loginForm = document.querySelector(".login-container");
let registerForm = document.querySelector(".register-container");

let registerFormDisplayBtn = document.querySelector("#registerFormBtn");
let loginFormDisplayBtn = document.querySelector("#loginFormBtn");

registerFormDisplayBtn.addEventListener("click", () => {
  loginForm.classList.toggle("display-none");
  registerForm.classList.toggle("display-block");
});
loginFormDisplayBtn.addEventListener("click", () => {
  loginForm.classList.toggle("display-none");
  registerForm.classList.toggle("display-block");
});

let loginBtn = document.querySelector("#loginBtn");
loginBtn.addEventListener("click", (e) => {
  e.preventDefault();
  if (validateLoginForm()) {
    window.location.assign("../HTML/FireBaseConnection.html");
  }
});

let registerBtn = document.querySelector("#registerBtn");
registerBtn.addEventListener("click", (e) => {
  e.preventDefault();
  if (validateForm()) {
    showAlert();
    setTimeout(() => {
      window.location.assign("../Home.html");
    }, 3000);
  }
});
/*-----------------------------------------------------------------*/

function validateLoginForm() {
  let username = document.getElementById("loginUsername").value;
  let password = document.getElementById("loginPassword").value;
  let usernameError = document.getElementById("loginUsernameError");
  let passwordError = document.getElementById("loginPasswordError");
  let isLoginFormValid = true;

  // Validate username
  if (username === "" || username !== "admin") {
    usernameError.innerHTML = "Please enter valid username";
    isLoginFormValid = false;
  } else {
    usernameError.innerHTML = "";
  }

  // Validate password
  if (password === "" || password !== "admin") {
    passwordError.innerHTML = "Please enter valid password";
    isLoginFormValid = false;
  } else {
    passwordError.innerHTML = "";
  }

  return isLoginFormValid;
}

/*------------------------------------------------------------------*/

function validateForm() {
  let firstName = document.getElementById("firstName").value;
  let lastName = document.getElementById("lastName").value;
  let username = document.getElementById("username").value;
  let password = document.getElementById("password").value;
  let birthdate = document.getElementById("birthdate").value;
  let firstNameError = document.getElementById("firstNameError");
  let lastNameError = document.getElementById("lastNameError");
  let usernameError = document.getElementById("usernameError");
  let passwordError = document.getElementById("passwordError");
  let birthdateError = document.getElementById("birthdateError");
  let isValid = true;

  // Validate first name
  if (firstName === "") {
    firstNameError.innerHTML = "Please enter your first name";
    isValid = false;
  } else {
    firstNameError.innerHTML = "";
  }

  // Validate last name
  if (lastName === "") {
    lastNameError.innerHTML = "Please enter your last name";
    isValid = false;
  } else {
    lastNameError.innerHTML = "";
  }

  // Validate username
  if (username === "") {
    usernameError.innerHTML = "Please enter your username";
    isValid = false;
  } else {
    usernameError.innerHTML = "";
  }

  // Validate password
  if (password === "") {
    passwordError.innerHTML = "Please enter your password";
    isValid = false;
  } else {
    passwordError.innerHTML = "";
  }

  // Validate birthdate
  if (birthdate === "") {
    birthdateError.innerHTML = "Please enter your birthdate";
    isValid = false;
  } else {
    birthdateError.innerHTML = "";
  }

  return isValid;
}
