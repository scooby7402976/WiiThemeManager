#ifndef _LANGS_H_
#define _LANGS_H_

/*	OLD SPECIAL CHARS (NOT NEEDED ANYMORE)
	�	\255	�	\250	�	\244	�	\207
	�	\240	�	\205	�	\204	�	\203
	�	\202	�	\212	�	\211	�	\210
	�	\241	�	\215	�	\213	�	\214
	�	\242	�	\225	�	\224	�	\223
	�	\243	�	\227	�	\201	�	\226
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
		"Zur�ck",//0
		"Verwalte NAND",
		"Bilder herunterladen",
		"Beenden",
		"Kan�le sortieren",
		"Verbinde...",
		"Lade bilder herunter...",
		"(beliebigen knopf dr�cken zum fortfahren)\n",
		"W�hle NAND modus:",
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
		"Zur�ck zum Wii-Men� (Quick-fix)", //25
		"SYSCONF aktualisieren",
		"Miis aktualisieren"
	},
	{ //FRENCH: thanks to Kalidor & Jimmyd45
		"Retour",//0
		"G�rer la NAND",
		"T�l�charger images",
		"Sortir",
		"Ranger les cha�nes",
		"Connexion...",
		"T�l�chargement des images en cours...",
		"(appuyez sur un bouton pour continuer)\n",
		"Choisissez le mode de la NAND:",
		"NAND r�elle",
		"NAND sur SD", //10
		"NAND sur USB",
		"D�marrage de la NAND r�elle...",
		"D�marrage de l'�mulation de la NAND sur SD...",
		"D�marrage de l'�mulation de la NAND sur USB...",
		"Configuration du jeu",
		"Sauvegarder les changements sur SD/USB",
		"R�essayer",
		"Oui",
		"Annuler",
		"d�sactiv�", //20
		"-",
		"R�cup�ration de la liste de jeux...",
		"Chargement...",
		"Aucune image t�l�charg�e.",
		"R�gler le probl�me du retour au menu Wii", //25
		"Actualiser SYSCONF",
		"Actualiser Miis"
	},
	{ //SPANISH
		"Atr�s",//0
		"Administrar NAND",
		"Descargar im�genes",
		"Salir",
		"Ordenar canales",
		"Conectando...",
		"Descargando im�genes...",
		"(pulsa un boton para continuar)\n",
		"Selecciona tipo de NAND:",
		"NAND real",
		"NAND en SD", //10
		"NAND en USB",
		"Cargando NAND real...",
		"Cargando emulaci�n NAND en SD...",
		"Cargando emulaci�n NAND en USB...",
		"Configuraci�n de juego",
		"Guardar cambios en SD/USB",
		"Reintentar",
		"S�",
		"Cancelar",
		"desactivado", //20
		"-",
		"Obteniendo lista de juegos...",
		"Cargando...",
		"No se descargaron im�genes.",
		"Arreglo Volver al men� de Wii", //25
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