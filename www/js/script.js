'use strict';

/* ---- Theme toggle with OS preference detection ---- */
const checkbox = document.getElementById('checkbox');
const body = document.body;

let theme = localStorage.getItem('theme');
if (!theme) {
    theme = window.matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
}
body.className = theme;
checkbox.checked = (theme === 'dark');

checkbox.addEventListener('change', () => {
    theme = (theme === 'dark') ? 'light' : 'dark';
    body.className = theme;
    localStorage.setItem('theme', theme);
});

/* ---- Mobile hamburger menu ---- */
const hamburger = document.querySelector('.hamburger');
const navList = document.querySelector('.navbar-nav');

if (hamburger) {
    hamburger.addEventListener('click', () => {
        const isOpen = navList.classList.toggle('open');
        hamburger.setAttribute('aria-expanded', isOpen);
    });

    navList.querySelectorAll('.nav-item').forEach(link => {
        link.addEventListener('click', () => {
            navList.classList.remove('open');
            hamburger.setAttribute('aria-expanded', 'false');
        });
    });
}

/* ---- Scrollspy using IntersectionObserver ---- */
const sections = document.querySelectorAll('main section[id]');
const navLinks = document.querySelectorAll('.navbar-nav a.nav-item:not(.lang-switch)');

const observerOptions = {
    root: null,
    rootMargin: '-20% 0px -75% 0px',
    threshold: 0
};

const observer = new IntersectionObserver((entries) => {
    entries.forEach(entry => {
        if (entry.isIntersecting) {
            navLinks.forEach(link => link.classList.remove('active'));
            const activeLink = document.querySelector(
                `.navbar-nav a[href="#${entry.target.id}"]`
            );
            if (activeLink) activeLink.classList.add('active');
        }
    });
}, observerOptions);

sections.forEach(section => observer.observe(section));

/* ---- Profile picture slideshow ---- */
(function () {
    const slider = document.getElementById('slider');
    if (!slider) return;

    const styleLink = document.querySelector('link[rel="stylesheet"][href*="style.css"]');
    const prefix = styleLink ? styleLink.getAttribute('href').replace('css/style.css', '') : '';

    const images = [
        prefix + 'img/pdp/pdp1.png',
        prefix + 'img/pdp/pdp2.png',
        prefix + 'img/pdp/pdp3.png',
        prefix + 'img/pdp/pdp4.png'
    ];
    const duration = 7000;
    let index = 0;

    images.forEach(src => { new Image().src = src; });

    function cycle() {
        slider.classList.add('fadeOut');
        setTimeout(() => {
            index = (index + 1) % images.length;
            slider.src = images[index];
            slider.classList.remove('fadeOut');
        }, 1500);
    }

    setInterval(cycle, duration);
})();
