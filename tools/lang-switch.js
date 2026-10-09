// lang-switch.js — the per-page language switch between the course's editions.
//
// Loaded by both books (book.toml and book-fr/book.toml, [output.html]
// additional-js). mdBook exposes `path_to_root` on every page; from it this
// script derives the page's edition root, decides which edition it is in
// (the French edition is the `fr/` directory of the published site), and
// renders one link to the same page in the other edition. Authoring tooling —
// the course's no-external-libraries rule governs student-visible C/C++, not
// this. Fails soft: if anything is missing, no switch is rendered and the
// page is unchanged.
(function () {
  "use strict";

  function counterpart() {
    if (typeof path_to_root !== "string") return null;

    // Edition root: the book root this page belongs to.
    var editionRoot = new URL(path_to_root === "" ? "./" : path_to_root, window.location.href);

    // The French edition is published as the `fr/` directory under the site
    // root; the English edition is the site root itself.
    var isFrench = /(^|\/)fr\/$/.test(editionRoot.pathname);
    var relPath = window.location.pathname.slice(editionRoot.pathname.length);
    if (relPath === "") relPath = "index.html";

    if (isFrench) {
      var siteRoot = new URL("..", editionRoot);
      return { href: new URL(relPath, siteRoot), label: "English" };
    }
    return { href: new URL("fr/" + relPath, editionRoot), label: "Français" };
  }

  function render() {
    var target;
    try {
      target = counterpart();
    } catch (e) {
      return; // fail soft
    }
    if (!target) return;

    var link = document.createElement("a");
    link.href = target.href.href;
    link.textContent = target.label;
    link.className = "lang-switch";
    link.setAttribute("aria-label", target.label === "English"
      ? "Read this page in English"
      : "Lire cette page en français");
    link.style.cssText = "margin-left:1em;font-size:0.9em;white-space:nowrap;";

    var bar = document.querySelector("#mdbook-menu-bar .right-buttons")
      || document.querySelector("#mdbook-menu-bar")
      || document.querySelector(".page");
    if (!bar) return;
    bar.appendChild(link);
  }

  if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", render);
  } else {
    render();
  }
})();
