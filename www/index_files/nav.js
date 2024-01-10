const navbarMenu = document.querySelector("#navigation #navbar-menu");
const hamburgerMenu = document.querySelector("#navigation .hamburger-menu");
const drops = Array.from(document.querySelectorAll("#navigation .drop"));
const dropdowns = Array.from(document.querySelectorAll("#navigation .cs-dropdown"));

hamburgerMenu.addEventListener('click', function() {
    navbarMenu.classList.toggle("open");
    hamburgerMenu.classList.toggle("clicked");
});

drops.forEach((drop, i) => {
  const dropdown = dropdowns[i] ;

  drop.addEventListener('click', function() {
      dropdown.classList.toggle("active");
  });

  drop.addEventListener('mouseover', function() {
      dropdown.classList.add("active");
  });

  drop.addEventListener('mouseout', function() {
      dropdown.classList.remove("active");
  });
  
});