/**** Gestion theme avec sauvegarde ****/
const checkbox = document.getElementById('checkbox');
const body = document.body;

let theme = localStorage.getItem('theme');

if (theme == null) {
	localStorage.setItem('theme', body.className);
	theme = body.className;
} else {
	body.className = theme;
	if (theme == 'dark') {
		checkbox.checked = true;
	}
}

checkbox.addEventListener('change', () => {
	if (theme == 'dark') {
		theme = 'light';
		body.classList.replace('dark', 'light');
		localStorage.setItem('theme', 'light');
	} else {
		theme = 'dark';
		body.classList.replace('light', 'dark');
  		localStorage.setItem('theme', 'dark');
	}
});

/************** Scrollspy **************/

const home = document.querySelector("#home");
const resume = document.querySelector("#resume");
const project = document.querySelector("#project");
const misc = document.querySelector("#misc");
const contact = document.querySelector("#contact");
const caca = 500;

window.addEventListener("scroll", () => {
  var windo = window.pageYOffset;
  //console.log(windo)
  if(resume.offsetTop <= windo && project.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#resume']").classList.add("active");
  }
  else if(project.offsetTop <= windo && misc.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#project']").classList.add("active");
  }
  else if(misc.offsetTop <= windo && contact.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#misc']").classList.add("active");
  }
  else if(contact.offsetTop <= windo+caca) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#contact']").classList.add("active");
  }
  else {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#home']").classList.add("active");
  }
})


/*
 * Gestion de formulaire
 * @author Mickael Martin Nevot
 */

const CONSIGNES_IMPRESSION = "Give your impression about this website, or contact me for anything.";
 
window.onload = loadJS;

function addEvent(element, event, func) {
	if (element.addEventListener) {
		element.addEventListener(event, func, false);
	} else {
		element.attachEvent("on" + event, func);
	}
}

function surligne(champ, erreur) {
	if(erreur) {
		champ.style.backgroundColor = "#fba";
	} else {
		champ.style.backgroundColor = "";
	}
}

function blurNom(event) {
	return verifNom(event.target);
}

function verifNom(champ) {
	var erreur = false;
	
	if(champ.value.length < 3 || champ.value.length > 12) {
		erreur = true;
	}
	
	surligne(champ, erreur);
	return !erreur;
}

function focusImpression(event) {
	var impression = document.getElementById("impression");
	
	if (impression.value == CONSIGNES_IMPRESSION) {
		impression.value = "";
	}
}

function blurImpression(event) {
	return verifImpression();
}

function verifImpression() {
	var impression = document.getElementById("impression");
	var erreur = false;
	
	if(impression.value.length < 3 || impression.value.length > 150 || impression.value == CONSIGNES_IMPRESSION) {
		erreur = true;
	}
	
	if (impression.value == "") {
		impression.value = CONSIGNES_IMPRESSION;
	}
	
	surligne(impression, erreur);
	return !erreur;
}

function effacerForm(event) {
	var impression = document.getElementById("impression");
	
	impression.value = CONSIGNES_IMPRESSION;
	
	verifImpression();
}

function verifForm(event) {
	var prenom = document.getElementById("prenom");
	var nom = document.getElementById("nom");
	var impression = document.getElementById("impression");
	var message = document.getElementById("message");
	
	var prenomOk = verifNom(prenom);
	var nomOk = verifNom(nom);
	var impressionOk = verifImpression(impression);
	
	if(prenomOk && nomOk && impressionOk) {
		message.innerHTML = "Message en cours d'envoi";
		return true;
	} else {
		event.preventDefault();
		message.innerHTML = "Veuillez remplir correctement tous les champs";
		return false;
	}
}

function loadJS() {
	var prenom = document.getElementById("prenom");
	var nom = document.getElementById("nom");
	var impression = document.getElementById("impression");
	var effacer = document.getElementById("effacer");
	var form = document.getElementById("form");
	
	impression.value = CONSIGNES_IMPRESSION;
	
	addEvent(prenom, "blur", blurNom);
	addEvent(nom, "blur", blurNom);
	addEvent(impression, "focus", focusImpression);
	addEvent(impression, "blur", blurImpression);
	addEvent(effacer, "click", effacerForm);
	addEvent(form, "submit", verifForm);
}

/* slideshow */
const imgArray = [
    '/img/pdp/pdp1.png',
	'/img/pdp/pdp2.png',
	'/img/pdp/pdp3.png',
    '/img/pdp/pdp4.png'],
    imgDuration = 7000;
let curIndex = 0;

function slideShow() {
    document.getElementById('slider').className += " fadeOut";
    setTimeout(function() {
        document.getElementById('slider').src = imgArray[curIndex];
        document.getElementById('slider').className = "pdp";
    },1500);
    curIndex++;
    if (curIndex == imgArray.length) { curIndex = 0; }
    setTimeout(slideShow, imgDuration);
}
slideShow();