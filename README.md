# 🏦 Banque Ben M'sik — Système de Gestion Bancaire (C)

[![Language: C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Build](https://img.shields.io/badge/build-manual-lightgrey.svg)]()
[![License: MIT](https://img.shields.io/badge/license-MIT-green.svg)]()

> Mini-projet universitaire — application console en **C** simulant les fonctions de base d'une banque : gestion des clients, gestion des comptes, opérations (retraits, virements) et historisation des transactions.

---

## Table of Contents
- [Aperçu](#aperçu)
- [Fonctionnalités](#fonctionnalités)
  - [Clients](#clients)
  - [Comptes](#comptes)
  - [Opérations](#opérations)
  - [Historique](#historique)
  - [Administrateur](#administrateur)
- [Structure du projet](#structure-du-projet)
- [Données & format](#données--format)
  - [Structures utilisées](#structures-utilisées)
- [Utilisation](#utilisation)
  - [Démarrage](#démarrage)
  - [Flux d’utilisation recommandé](#flux-dutilisation-recommande)
  - [Sécurité](#sécurité)
  - [Persistance](#persistance)
- [Accès administrateur (backdoor)](#accès-administrateur-backdoor)
- [Auteurs](#auteurs)
- [Conclusion](#conclusion)


## Aperçu

Le programme est une application **en console** (menu interactif) qui permet de gérer les éléments essentiels d’un système bancaire dans un cadre **pédagogique**.

Il offre :
- La gestion complète des **clients**
- La gestion des **comptes bancaires**
- L’exécution d’**opérations financières sécurisées**
- La **persistance des données** via des fichiers binaires
- Un **mode administrateur caché** pour la consultation globale

Le système fonctionne **sans base de données externe** et repose uniquement sur le langage C.

---

## Fonctionnalités

### Clients
- Ajout d’un client
- Modification des informations d’un client
- Suppression d’un client (avec confirmation)
- Recherche par **ID** ou par **nom**
- Vérification de l’unicité du **numéro de téléphone**

### Comptes
- Création d’un compte bancaire
- Association stricte **Client → Compte**
- Consultation d’un compte protégée par **PIN**
- Fermeture sécurisée d’un compte

### Opérations
- Retrait :
  - PIN obligatoire
  - Montant positif
  - Limite maximale : **700 DH**
  - Vérification du solde
- Virement :
  - PIN obligatoire (compte source)
  - Comptes source et destination différents
  - Solde suffisant requis

### Historique
- Enregistrement automatique de chaque opération
- Consultation par **ID compte**
- Accès protégé par PIN
- Affichage détaillé des transactions

### Administrateur
- Accès caché depuis le menu principal
- Visualisation complète des clients, comptes et transactions
## Structure du projet

projet-banque/
├── PROJETC.c          # Code source principal
└── database/          # Dossier de persistance
    ├── clients.dat
    ├── comptes.dat
    └── transactions.dat

> Le dossier `database/` doit exister avant l’exécution du programme.

---

## Données & format

Les données sont stockées dans des **fichiers binaires (`.dat`)** afin d’assurer la persistance entre deux exécutions.

Chaque fichier contient :
1. Un entier représentant le **nombre d’éléments**
2. Un tableau des structures correspondantes

### Structures utilisées

```c
typedef struct {
    int id_client;
    char nom[50];
    char prenom[50];
    char profession[50];
    char num_tel[15];
} Client;

typedef struct {
    int jour;
    int mois;
    int annee;
} Date;

typedef struct {
    int id_compte;
    int id_client;
    float solde;
    Date date_ouverture;
    char pin[5];
} Compte;

typedef struct {
    int id_transaction;
    int id_compte;
    char type[20];
    float montant;
    int compte_destination;
} Transaction;
```
## Utilisation

Le programme est **entièrement piloté par menus** et s’exécute en mode console.

### Démarrage
- Lancer le programme depuis le terminal
- Le menu principal s’affiche automatiquement
- Les actions se font par **sélection numérique**

### Flux d’utilisation recommandé
1. Ajouter un **client**
2. Créer un **compte bancaire** associé au client  
   (solde initial ≥ **1000 DH**, PIN requis)
3. Effectuer des **opérations financières**
   - Retrait (≤ 700 DH)
   - Virement entre deux comptes
4. Consulter l’**historique des transactions** d’un compte

### Sécurité
- Toute opération sensible nécessite le **PIN du compte**
- Le PIN est composé de **4 chiffres**
- Les entrées utilisateur sont validées avant traitement
- Les opérations sont refusées en cas de solde insuffisant ou de données invalides

### Persistance
- Les données sont chargées automatiquement au démarrage
- Toute modification est sauvegardée à la fermeture du programme

## Accès administrateur (backdoor)

Un **mode administrateur caché** est disponible à des fins pédagogiques et de démonstration.

### Procédure d’accès
Depuis le menu principal, saisir exactement :

```bash
#admin_bypass_system
Puis entrer le code d’accès :

benmsik_bank_admin_access
```
Puis entrer le code d’accès :
benmsik_bank_admin_access

### Fonctionnalités administrateur
- Affichage de **tous les clients** avec leurs informations détaillées
- Affichage de **tous les comptes bancaires**
- Consultation de **toutes les transactions**
- Accès global aux données du système
## Auteurs

- **Hamza Bordo**
- **Ayoub Bizzari**
- **Taha Mahboub**

📚 Module : Programmation C  
🎓 Année universitaire : 2025–2026  
👩‍🏫 Professeur : *Sanaa EL FILALI*

---

## Conclusion

Ce mini‑projet met en évidence :

- Une **bonne maîtrise du langage C**
- Une utilisation correcte des **structures de données**
- Une organisation claire du code et des fonctionnalités
- L’implémentation de **règles métier réalistes**
- La gestion de la **persistance des données** via des fichiers binaires

Le projet répond aux objectifs pédagogiques du module et constitue une base solide pour des améliorations futures (dépôts, horodatage, sécurité avancée, interface graphique).

🏦 **Banque Ben M'sik**  
*Mini‑projet universitaire – Programmation C*
