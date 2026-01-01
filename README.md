\# Gestion des Clients et des Comptes Bancaires (C)



Mini‑projet en langage C pour la gestion simple d’un système bancaire en mode console.



Fonctionnalités principales :



\- Gestion des \*\*clients\*\* : ajout, modification, suppression, recherche.

\- Gestion des \*\*comptes\*\* : création, consultation, fermeture.

\- Gestion des \*\*opérations\*\* : versement, retrait (limité à 700 DH), virement.

\- \*\*Historique des transactions\*\* (versements, retraits, virements).

\- \*\*Sécurité par PIN\*\* pour les opérations sensibles.

\- \*\*Sauvegarde automatique\*\* des données dans des fichiers binaires + export en `.txt`.



---



\## 1. Structure générale



Le projet est constitué principalement d’un seul fichier source C :



\- `main.c` – contient :

&nbsp; - les structures `Client`, `Compte`, `Transaction`

&nbsp; - les menus (principal, gestion des clients, comptes, opérations)

&nbsp; - les fonctions d’ajout/modification/suppression/recherche

&nbsp; - les opérations financières (versement, retrait, virement)

&nbsp; - la gestion de l’historique de transactions

&nbsp; - la sauvegarde/chargement des données



À l’exécution, le programme utilise les dossiers suivants :



\- `database/`  

&nbsp; - `clients.dat`  

&nbsp; - `comptes.dat`  

&nbsp; - `transactions.dat`  



\- `database/backdoor acces/`  

&nbsp; - `clients.txt`  

&nbsp; - `comptes.txt`  

&nbsp; - `transactions.txt`  



Les dossiers sont créés automatiquement si besoin.  

Les fichiers `.dat` sont utilisés par le programme, les `.txt` sont lisibles dans un éditeur de texte.



---



\## 2. Fonctionnalités et règles métier



\### 2.1. Gestion des clients



\- \*\*Ajouter un client\*\*

&nbsp; - `id\_client` unique

&nbsp; - `nom`, `prenom` : sans chiffres

&nbsp; - `num\_tel` : 10 à 14 chiffres, unique dans le système

&nbsp; - `profession` : texte simple



\- \*\*Modifier un client\*\*

&nbsp; - Recherche par `id\_client`

&nbsp; - Modification possible de : nom, prénom, profession, téléphone



\- \*\*Supprimer un client\*\*

&nbsp; - Suppression par `id\_client`

&nbsp; - Si le client possède des comptes, avertissement et confirmation obligatoire



\- \*\*Rechercher un client\*\*

&nbsp; - Par `id\_client`

&nbsp; - Ou par `nom`

&nbsp; - Affiche aussi le nombre de comptes du client



\### 2.2. Gestion des comptes



\- \*\*Créer un compte\*\*

&nbsp; - `id\_compte` unique

&nbsp; - Associé à un `id\_client` existant

&nbsp; - Solde initial \*\*≥ 1000 DH\*\*

&nbsp; - Date d’ouverture au format `JJ/MM/AAAA`

&nbsp; - Création d’un \*\*code PIN à 4 chiffres\*\*



\- \*\*Consulter les comptes\*\*

&nbsp; - Afficher tous les comptes

&nbsp; - Ou afficher un compte précis (avec client, solde, date, info PIN active)



\- \*\*Fermer un compte\*\*

&nbsp; - Recherche par `id\_compte`

&nbsp; - Vérification du PIN

&nbsp; - Confirmation de suppression



\### 2.3. Opérations financières



\- \*\*Versement\*\*

&nbsp; - Ajout d’un montant positif au solde

&nbsp; - Enregistre une transaction de type `Versement`



\- \*\*Retrait\*\*

&nbsp; - Demande du PIN

&nbsp; - Montant > 0 et \*\*≤ 700 DH\*\*

&nbsp; - Vérifie que le solde est suffisant

&nbsp; - Enregistre une transaction de type `Retrait`



\- \*\*Virement\*\*

&nbsp; - Compte source et compte destination différents

&nbsp; - Vérification du PIN sur le compte source

&nbsp; - Vérifie que le solde du compte source est suffisant

&nbsp; - Enregistre deux transactions : une sur chaque compte



\### 2.4. Historique des transactions



Menu \*\*Historique des transactions\*\* (dans Gestion des opérations) :



\- Afficher \*\*toutes\*\* les transactions enregistrées

\- Afficher seulement les transactions d’un \*\*compte donné\*\*



Chaque transaction mémorise :



\- `id\_transaction`

\- `id\_compte`

\- `type` (Retrait / Versement / Virement)

\- `montant`

\- `date` (date + heure)

\- `details`

\- `compte\_destination` pour les virements



---



\## 3. Sauvegarde et stockage des données



\### 3.1. Sauvegarde binaire



À la fin du programme, les données sont sauvegardées automatiquement dans :



\- `database/clients.dat`  

\- `database/comptes.dat`  

\- `database/transactions.dat`



Au démarrage, le programme recharge ces fichiers (silencieusement) s’ils existent.



\### 3.2. Export texte



En plus, des fichiers texte lisibles sont générés dans :



\- `database/backdoor acces/clients.txt`  

\- `database/backdoor acces/comptes.txt`  

\- `database/backdoor acces/transactions.txt`  



Ces fichiers contiennent les données au format \*\*CSV simple\*\* (`;` comme séparateur) pour consultation directe dans un éditeur ou tableur.



---



\## 4. Compilation et exécution



\### 4.1. Windows (GCC via MinGW ou similar)



1\. Ouvrir un terminal (PowerShell, par exemple).

2\. Se placer dans le dossier du projet :



&nbsp;  ```powershell

&nbsp;  cd "C:\\Users\\VOTRE\_NOM\\Desktop\\banque-projet"

