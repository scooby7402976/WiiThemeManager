#ifndef _LANGS_H_
#define _LANGS_H_

/*	OLD SPECIAL CHARS (NOT NEEDED ANYMORE)
	¡	\255	¿	\250	ñ	\244	ç	\207
	á	\240	à	\205	ä	\204	â	\203
	é	\202	è	\212	ë	\211	ê	\210
	í	\241	ì	\215	ï	\213	î	\214
	ó	\242	ò	\225	ö	\224	ô	\223
	ú	\243	ù	\227	ü	\201	û	\226
*/

const char *texts[6][28]={
	{ //ENGLISH
		"Back",//0
		"Manage NAND",
		"Download images",
		"Exit",
		"Sort channels",
		"Connecting...",
		"Downloading images...",
		"(press any button to continue)\n",
		"Select Theme Folder Device :",
		"Sd Card",
		"Usb Device", //10
		"Neek NAND",
		"Unable to Detect a Sd or USB device . Exiting .....   ",
		"Unable to Detect Usb Device . Press any button to Exit .",
		"Unable to Detect Usb Device . Checking for Sd Card .",
		"Game configuration",
		"Save changes on SD/USB",
		"Retry",
		"Yes",
		"Cancel",
		"disabled", //20
		"-",
		"Getting game list...",
		"Loading...",
		"No images were downloaded.",
		"Return to Wii Menu quick-fix", //25
		"No Themes Found .",
		"Unable to Initiate Wii File System . Exiting .....   "
	},
	{ //GERMAN: thanks to FIX94 & Liberty
		"Zurück",//0
		"Verwalte NAND",
		"Bilder herunterladen",
		"Beenden",
		"Kanäle sortieren",
		"Verbinde...",
		"Lade bilder herunter...",
		"(beliebigen knopf drücken zum fortfahren)\n",
		"Wähle NAND modus:",
		"Echtes NAND",
		"SD NAND", //10
		"USB NAND",
		"Starte echtes NAND...",
		"Starte SD NAND emulation...",
		"Starte USB NAND emulation...",
		"Spieleinstellungen",
		"Einstellungen auf SD/USB speichern",
		"Wiederholen",
		"Ja",
		"Abbrechen",
		"deaktiviert", //20
		"-",
		"Bekomme spieleliste...",
		"Lade...",
		"Es wurden keine bilder heruntergeladen.",
		"Zurück zum Wii-Menü (Quick-fix)", //25
		"SYSCONF aktualisieren",
		"Miis aktualisieren"
	},
	{ //FRENCH: thanks to Kalidor & Jimmyd45
		"Retour",//0
		"Gérer la NAND",
		"Télécharger images",
		"Sortir",
		"Ranger les chaînes",
		"Connexion...",
		"Téléchargement des images en cours...",
		"(appuyez sur un bouton pour continuer)\n",
		"Choisissez le mode de la NAND:",
		"NAND réelle",
		"NAND sur SD", //10
		"NAND sur USB",
		"Démarrage de la NAND réelle...",
		"Démarrage de l'émulation de la NAND sur SD...",
		"Démarrage de l'émulation de la NAND sur USB...",
		"Configuration du jeu",
		"Sauvegarder les changements sur SD/USB",
		"Réessayer",
		"Oui",
		"Annuler",
		"désactivé", //20
		"-",
		"Récupération de la liste de jeux...",
		"Chargement...",
		"Aucune image téléchargée.",
		"Régler le problème du retour au menu Wii", //25
		"Actualiser SYSCONF",
		"Actualiser Miis"
	},
	{ //SPANISH
		"Atrás",//0
		"Administrar NAND",
		"Descargar imágenes",
		"Salir",
		"Ordenar canales",
		"Conectando...",
		"Descargando imágenes...",
		"(pulsa un boton para continuar)\n",
		"Selecciona tipo de NAND:",
		"NAND real",
		"NAND en SD", //10
		"NAND en USB",
		"Cargando NAND real...",
		"Cargando emulación NAND en SD...",
		"Cargando emulación NAND en USB...",
		"Configuración de juego",
		"Guardar cambios en SD/USB",
		"Reintentar",
		"Sí",
		"Cancelar",
		"desactivado", //20
		"-",
		"Obteniendo lista de juegos...",
		"Cargando...",
		"No se descargaron imágenes.",
		"Arreglo Volver al menú de Wii", //25
		"Actualizar SYSCONF",
		"Actualizar Miis"
	},
	{ //ITALIAN: thanks to vejta
		"Indietro",//0
		"Gestione NAND",
		"Download immagini",
		"Esci",
		"Ordina canali",
		"In connessione...",
		"Sto scaricando le immagini...",
		"(premi un pulsante qualsiasi per continuare)\n",
		"Selezionare il tipo di NAND:",
		"NAND reale",
		"NAND su SD", //10
		"NAND su USB",
		"Caricamento NAND reale...",
		"Caricamento NAND su SD (emulazione)...",
		"Caricamento NAND su USB (emulazione)...",
		"Configurazione gioco",
		"Salva le modifiche su SD/USB",
		"ritenta",
		"Si",
		"Annulla",
		"disattivato", //20
		"-",
		"Caricamento lista giochi...",
		"Caricamento...",
		"Non sono state scaricate immagini.",
		"Ritornare al Menu della Wii (quick-fix)", //25
		"Aggiornare SYSCONF",
		"Aggiornare Mii"
	},
	{ //DUTCH: thanks to digdug3
		"Terug",//0
		"Beheer NAND",
		"Download plaatjes",
		"Quit",
		"Sorteer kanalen",
		"Verbinden...",
		"Downloaden plaatjes...",
		"(druk op een toets om verder te gaan)\n",
		"Selecteer NAND mode:",
		"Echte NAND",
		"SD NAND", //10
		"USB NAND",
		"Starten echte NAND...",
		"Starten SD NAND emulatie...",
		"Starten USB NAND emulatie...",
		"Spel configuratie",
		"Bewaar wijzigingen op SD/USB",
		"Opnieuw",
		"Ja",
		"Annuleer",
		"uitgeschakeld", //20
		"-",
		"Ophalen spellijst...",
		"Laden...",
		"Geen plaatjes gedownload.",
		"Terug naar Wii Menu quick-fix", //25
		"Update SYSCONF",
		"Update Miis"
	}
};

#endif