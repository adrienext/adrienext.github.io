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

let accueil = document.querySelector("#accueil");
let cv = document.querySelector("#cv");
let projet = document.querySelector("#projet");
let divers = document.querySelector("#divers");
let formulaire = document.querySelector("#formulaire");

window.addEventListener("scroll", () => {
  var windo = window.pageYOffset;
  if(cv.offsetTop <= windo && projet.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#cv']").classList.add("active");
  }
  else if(projet.offsetTop <= windo && divers.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#projet']").classList.add("active");
  }
  else if(divers.offsetTop <= windo && formulaire.offsetTop > windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#divers']").classList.add("active");
  }
  else if(formulaire.offsetTop <= windo) {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#formulaire']").classList.add("active");
  }
  else {
    document.querySelector(".active").classList.remove("active");
    document.querySelector("[href='#accueil']").classList.add("active");
  }
})


/*
 * Gestion de formulaire
 * @author Mickael Martin Nevot
 */

const CONSIGNES_IMPRESSION = "Donnez vos impressions sur le site";
 
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