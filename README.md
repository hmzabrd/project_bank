# 🏦 Système de Gestion Bancaire en C  
**Banque Ben M'sik – Mini-Projet Universitaire**

Mini-projet universitaire développé en **langage C**, simulant un **système bancaire complet en console**, avec gestion des clients, des comptes, des opérations financières, persistance des données et un mode administrateur sécurisé.

---

## 📌 Table des matières

1. Présentation générale  
2. Fonctionnalités implémentées  
3. Architecture du projet  
4. Structures de données  
5. Gestion des clients  
6. Gestion des comptes  
7. Gestion des opérations  
8. Historique des transactions  
9. Mode Administrateur  
10. Sécurité et validations  
11. Persistance des données  
12. Compilation et exécution  
13. Limites connues  
14. Auteurs  

---

## 🎯 Présentation générale

Ce projet implémente un **système de gestion bancaire** permettant :

- La gestion des **clients**
- La gestion des **comptes bancaires**
- L’exécution d’**opérations financières sécurisées**
- La **traçabilité complète** via un historique de transactions
- La **sauvegarde automatique** des données dans des fichiers binaires
- Un **mode administrateur caché**

Le programme fonctionne **entièrement en console**, sans base de données externe.

---

## 🚀 Fonctionnalités implémentées

### ✅ Gestion des clients
- Ajouter un client
- Modifier un client
- Supprimer un client (avec confirmation)
- Rechercher un client par **ID** ou **Nom**
- Vérification de l’unicité du **numéro de téléphone**

### ✅ Gestion des comptes
- Création d’un compte bancaire
- Consultation sécurisée par **PIN**
- Fermeture d’un compte
- Relation stricte **Client → Comptes**

### ✅ Opérations financières
- Retrait (limité à 700 DH)
- Virement entre deux comptes
- Enregistrement automatique des transactions

### ✅ Historique des transactions
- Consultation par compte
- Accès protégé par PIN
- Affichage détaillé de chaque transaction

### ✅ Mode administrateur
- Accès caché et sécurisé
- Visualisation complète des données du système

---

## 📁 Architecture du projet

projet-banque/
│
├── PROJETC.c # Code source principal
│
└── database/
├── clients.dat # Données clients (binaire)
├── comptes.dat # Données comptes (binaire)
└── transactions.dat # Données transactions (binaire)

yaml
Copy code

📌 Le dossier `database/` doit exister avant l’exécution.

---

## 🧱 Structures de données

### Client
```c
typedef struct {
    int id_client;
    char nom[50];
    char prenom[50];
    char profession[50];
    char num_tel[15];
} Client;
Date
c
Copy code
typedef struct {
    int jour;
    int mois;
    int annee;
} Date;
Compte
c
Copy code
typedef struct {
    int id_compte;
    int id_client;
    float solde;
    Date date_ouverture;
    char pin[5];
} Compte;
Transaction
c
Copy code
typedef struct {
    int id_transaction;
    int id_compte;
    char type[20];
    float montant;
    int compte_destination;
} Transaction;
👥 Gestion des clients
➕ Ajouter un client
ID client unique

Nom & prénom : sans chiffres

Téléphone : 10 à 14 chiffres, unique

Profession : texte libre

✏️ Modifier un client
Nom

Prénom

Profession

Numéro de téléphone (unicité vérifiée)

❌ Supprimer un client
Avertissement si le client possède des comptes

Confirmation obligatoire (O/N)

🔍 Rechercher un client
Par ID

Par Nom

Affichage du nombre de comptes associés

💳 Gestion des comptes
➕ Créer un compte
ID compte unique

Client existant obligatoire

Solde initial ≥ 1000 DH

Date d’ouverture valide

PIN à 4 chiffres, avec confirmation

Création automatique d’une transaction "Ouverture"

👁️ Consultation d’un compte
Authentification par PIN

Affichage :

Informations du client

Solde

Date d’ouverture

🔒 Fermeture d’un compte
Authentification par PIN

Confirmation utilisateur

Suppression définitive du compte

💸 Gestion des opérations
🔻 Retrait
PIN obligatoire

Montant > 0

Limite maximale : 700 DH

Solde suffisant requis

Transaction enregistrée automatiquement

🔁 Virement
PIN obligatoire (compte source)

Comptes source et destination différents

Solde suffisant

Création de 2 transactions (source + destination)

📜 Historique des transactions
Consultation par ID compte

Authentification par PIN

Affichage :

ID transaction

Type

Montant

Compte lié (pour les virements)

🔐 Mode Administrateur
Accès
Depuis le menu principal, saisir :

bash
Copy code
#admin_bypass_system
Puis entrer le code :

nginx
Copy code
benmsik_bank_admin_access
Fonctionnalités admin
Voir tous les clients (détaillé)

Voir tous les comptes (détaillé)

Voir toutes les transactions

Accès complet aux données

⚠️ Le PIN est visible en mode administrateur (choix pédagogique).

🔒 Sécurité et validations
Validations implémentées
Unicité :

ID client

ID compte

Téléphone

Formats :

Nom / Prénom : lettres uniquement

Téléphone : chiffres uniquement

PIN : exactement 4 chiffres

Date : valide (jour / mois / année)

Règles métier :

Solde minimum à la création : 1000 DH

Retrait ≤ 700 DH

Montants positifs

Solde suffisant pour retrait / virement

💾 Persistance des données
Sauvegarde automatique
À la fermeture du programme

Fichiers binaires :

clients.dat

comptes.dat

transactions.dat

Chargement automatique
Au démarrage du programme

Restauration complète de l’état précédent

Si fichiers absents → base vide

⚙️ Compilation et exécution
Compilation
bash
Copy code
gcc -Wall -Wextra -o banque PROJETC.c
Exécution
bash
Copy code
./banque
⚠️ Limites connues
Pas de versement (dépôt) direct

Pas de date/heure réelle des transactions

Pas de base de données SQL

Pas de gestion des tentatives PIN multiples

Interface console uniquement

Ces limites sont volontaires et adaptées au cadre académique.

👨‍💻 Auteurs
Hamza Bordo

Ayoub Bizzari

Taha Mahboub

📚 Module : Programmation C
🎓 Année universitaire : 2025–2026
👩‍🏫 Professeur : Sanaa EL FILALI

🎓 Conclusion
Ce projet démontre :

Une maîtrise solide du langage C

Une bonne utilisation des structures

Une architecture claire et modulaire

Des règles métier réalistes

Une persistance fiable des données

🏦 Banque Ben M'sik
Mini-projet universitaire – Programmation C
