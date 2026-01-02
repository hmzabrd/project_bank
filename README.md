# Système de Gestion Bancaire en C
**Banque Ben M'sik - Mini-Projet Universitaire**

Système complet de gestion bancaire en langage C avec interface console, développé dans le cadre d'un projet universitaire.

---

## 📋 Table des matières

1. [Vue d'ensemble](#vue-densemble)
2. [Architecture du projet](#architecture-du-projet)
3. [Fonctionnalités principales](#fonctionnalités-principales)
4. [Mode Administrateur](#mode-administrateur)
5. [Sécurité et validations](#sécurité-et-validations)
6. [Persistance des données](#persistance-des-données)
7. [Compilation et exécution](#compilation-et-exécution)
8. [Spécifications techniques](#spécifications-techniques)

---

## 🎯 Vue d'ensemble

Mini-projet en langage C implémentant un système bancaire complet avec :

- ✅ **Gestion des clients** : CRUD complet (Create, Read, Update, Delete)
- ✅ **Gestion des comptes** : Création, consultation, fermeture
- ✅ **Opérations financières** : Versement, retrait (limité à 700 DH), virement
- ✅ **Historique des transactions** : Traçabilité complète de toutes les opérations
- ✅ **Sécurité par PIN** : Authentification à 4 chiffres avec blocage après 3 tentatives
- ✅ **Sauvegarde automatique** : Fichiers binaires + exports CSV lisibles
- ✅ **Mode Administrateur** : Statistiques avancées et accès complet aux données

---

## 📁 Architecture du projet

### Structure des fichiers
```
projet-banque/
│
├── PROJETC.c                    # Fichier source principal (~1400 lignes)
│
└── database/                    # Créé automatiquement à l'exécution
    ├── clients.dat              # Données clients (format binaire)
    ├── comptes.dat              # Données comptes (format binaire)
    ├── transactions.dat         # Données transactions (format binaire)
    │
    └── backdoor acces/          # Exports lisibles
        ├── clients.txt          # Export CSV des clients
        ├── comptes.txt          # Export CSV des comptes
        └── transactions.txt     # Export CSV des transactions
```

### Structures de données

#### Structure Client
```c
typedef struct {
    int id_client;
    char nom[50];
    char prenom[50];
    char profession[50];
    char num_tel[15];
} Client;
```

#### Structure Compte
```c
typedef struct {
    int id_compte;
    int id_client;              // Propriétaire du compte
    float solde;
    char date_ouverture[20];
    char pin[5];                // PIN à 4 chiffres
} Compte;
```

#### Structure Transaction
```c
typedef struct {
    int id_transaction;
    int id_compte;
    char type[20];              // "Versement", "Retrait", "Virement"
    float montant;
    char date[30];              // Date et heure complètes
    char details[150];          // Description de l'opération
    int compte_destination;     // Pour les virements, ou -1
} Transaction;
```

---

## 🚀 Fonctionnalités principales

### 1. Gestion des clients

#### Ajouter un client
- **ID unique** obligatoire
- **Nom/Prénom** : Sans chiffres
- **Téléphone** : 10-14 chiffres, **unique dans le système**
- **Profession** : Texte libre

#### Modifier un client
- Recherche par `id_client`
- Modification des champs individuels :
  - Nom
  - Prénom
  - Profession
  - Numéro de téléphone (avec vérification d'unicité)

#### Supprimer un client
- Suppression par `id_client`
- ⚠️ **Avertissement si le client possède des comptes**
- **Double confirmation** obligatoire (O/N)

#### Rechercher un client
- Par **ID** (recherche directe)
- Par **Nom** (recherche séquentielle)
- Affiche le nombre de comptes associés

---

### 2. Gestion des comptes

#### Créer un compte
- **ID compte unique** obligatoire
- Associé à un **client existant**
- **Solde initial ≥ 1000 DH** (contrainte métier)
- **Date d'ouverture** : Format JJ/MM/AAAA strict
- **Code PIN à 4 chiffres** :
  - Saisie avec confirmation
  - Validation (uniquement des chiffres)
- 🔄 **Création automatique** d'une transaction "Versement initial"

#### Consulter un compte
- **Authentification PIN obligatoire**
- 3 tentatives maximum
- Affichage :
  - Informations du compte
  - Propriétaire (nom complet)
  - Solde actuel
  - Date d'ouverture
  - Statut sécurité (PIN actif)

#### Fermer un compte
- **Authentification PIN obligatoire**
- Affichage du solde final
- **Confirmation utilisateur** (O/N)
- Suppression définitive

---

### 3. Opérations financières

#### 💰 Versement
- **Aucune authentification requise**
- **Aucune limite de montant**
- Montant doit être positif
- 🔄 Enregistrement automatique d'une transaction

#### 💸 Retrait
- ✅ **Authentification PIN obligatoire** (3 tentatives max)
- ✅ **Limite : 700 DH maximum par opération**
- ✅ Vérification du solde suffisant
- 🔄 Enregistrement automatique d'une transaction

#### 🔄 Virement
- ✅ **Authentification PIN obligatoire** (compte source)
- ✅ Comptes source et destination **doivent être différents**
- ✅ Vérification du solde suffisant
- 🔄 **Création de 2 transactions** :
  - Transaction débit sur compte source
  - Transaction crédit sur compte destination
  - Chaque transaction référence l'autre compte

---

### 4. Historique des transactions

#### Consultation de l'historique
- **Authentification PIN obligatoire**
- Historique par compte individuel (pas de vue globale)
- Affichage chronologique complet

#### Informations affichées
- ID transaction
- Type d'opération (Versement/Retrait/Virement)
- Montant
- Date et heure exactes
- Détails de l'opération
- Compte lié (pour les virements)

#### Auto-logging
Toutes les opérations génèrent automatiquement des transactions :
- **Ouverture de compte** → Transaction "Versement initial"
- **Versement** → Transaction de crédit
- **Retrait** → Transaction de débit
- **Virement** → 2 transactions (source + destination)

---

## 🔐 Mode Administrateur

### Accès administrateur

Le système dispose d'un **mode administrateur caché** accessible via deux codes secrets :

#### 1️⃣ Accès depuis le menu principal
Saisir le code : `#admin_bypass_system`

#### 2️⃣ Authentification backdoor
Code d'accès : `benmsik_bank_admin_access`

### Fonctionnalités administrateur

Une fois authentifié, accès à un menu dédié avec :

#### 📊 Statistiques globales
```
--- CLIENTS ---
Total: 5 clients
Avec comptes: 4
Sans comptes: 1

--- COMPTES ---
Total: 7 comptes
Solde total: 45,300.00 DH
Solde moyen: 6,471.43 DH
Solde minimum: 1,000.00 DH
Solde maximum: 15,800.00 DH

--- TRANSACTIONS ---
Total: 23 transactions
Versements: 9 (Total: 28,500 DH)
Retraits: 8 (Total: 3,200 DH)
Virements: 6 (Total: 8,400 DH)
```

#### 👥 Voir tous les clients (détaillé)
- Liste complète avec toutes les informations
- Nombre de comptes par client

#### 💳 Voir tous les comptes (détaillé)
- Liste complète avec propriétaires
- Soldes et dates d'ouverture
- Nombre de transactions par compte
- **PINs masqués** (****) pour la sécurité

#### 📝 Voir toutes les transactions (détaillé)
- Historique complet du système
- Tous les comptes confondus

#### 📋 Fiche client complète
- Informations du client
- Liste de tous ses comptes
- Solde total cumulé

#### 🔍 Fiche compte complète
- Informations du compte
- Propriétaire
- **PIN visible** (privilège admin)
- Historique complet des transactions

#### ℹ️ Informations système
- Nombre total de clients/comptes/transactions
- Prochain ID transaction
- Solde total de tous les comptes
- Liste des fichiers de données

---

## 🔒 Sécurité et validations

### Authentification PIN

#### Opérations protégées par PIN
✅ **PIN requis** pour :
- Consultation de compte
- Retrait
- Virement (compte source)
- Fermeture de compte
- Historique des transactions

❌ **PIN NON requis** pour :
- Versement (dépôt d'argent)

#### Système de sécurité
- **3 tentatives maximum**
- Blocage temporaire après échec
- PIN masqué à l'affichage (****)
- Confirmation requise lors de la création

### Validations métier

#### Contraintes d'unicité
- ✅ **ID Client** : Unique dans le système
- ✅ **ID Compte** : Unique dans le système
- ✅ **Numéro de téléphone** : Unique (un numéro = un client)

#### Validations de format
- ✅ **Nom/Prénom** : Sans chiffres
- ✅ **Téléphone** : 10-14 chiffres, uniquement numériques
- ✅ **Date** : Format JJ/MM/AAAA strict
  - Jour : 1-31
  - Mois : 1-12
  - Année : 1900-2100
- ✅ **PIN** : Exactement 4 chiffres

#### Contraintes financières
- ✅ **Solde minimum** : 1000 DH à la création
- ✅ **Limite retrait** : 700 DH maximum par opération
- ✅ **Montants positifs** : Obligatoires pour toutes les opérations
- ✅ **Solde suffisant** : Vérifié avant retrait/virement

#### Règles de relation
- ✅ **1 Client** peut avoir **plusieurs Comptes**
- ✅ **1 Compte** appartient à **1 seul Client**
- ✅ **1 Compte** peut avoir **plusieurs Transactions**
- ✅ **Compte source ≠ Compte destination** pour les virements

---

## 💾 Persistance des données

### Système de persistance "silencieuse"

Le système utilise une approche **automatique et transparente** :

#### Au démarrage
- ✅ Chargement automatique des fichiers `.dat`
- ✅ **Aucun message** affiché à l'utilisateur
- ✅ Restauration complète de l'état précédent
- ✅ Si fichiers absents → initialisation vide

#### À la fermeture
- ✅ Sauvegarde automatique des fichiers `.dat`
- ✅ Export automatique des fichiers `.txt` (CSV)
- ✅ **Aucune intervention utilisateur**
- ✅ Création automatique des dossiers si nécessaire

### Format binaire (`.dat`)

**Avantages** :
- ⚡ Chargement/sauvegarde rapide
- 📦 Taille optimisée
- 🔒 Format non éditable manuellement

**Fichiers** :
- `database/clients.dat`
- `database/comptes.dat`
- `database/transactions.dat`

### Format texte (`.txt` / CSV)

**Avantages** :
- 📖 Lisible dans un éditeur de texte
- 📊 Importable dans un tableur (Excel, LibreOffice)
- 🔍 Inspection facile des données

**Format** :
- Séparateur : `;` (point-virgule)
- Encodage : UTF-8
- Première ligne : En-têtes des colonnes

**Fichiers** :
- `database/backdoor acces/clients.txt`
- `database/backdoor acces/comptes.txt`
- `database/backdoor acces/transactions.txt`

**Exemple** (`clients.txt`) :
```
ID_CLIENT;NOM;PRENOM;PROFESSION;TELEPHONE
101;Alami;Hassan;Ingenieur;0612345678
102;Bennani;Fatima;Medecin;0623456789
```

---

## ⚙️ Compilation et exécution

### Windows (MinGW / GCC)

#### 1. Compiler
```cmd
gcc -o banque.exe PROJETC.c
```

#### 2. Exécuter
```cmd
banque.exe
```

### Linux / macOS

#### 1. Compiler
```bash
gcc -o banque PROJETC.c
```

#### 2. Exécuter
```bash
./banque
```

### Compilation avec warnings
```bash
gcc -Wall -Wextra -o banque PROJETC.c
```

---

## 🛠️ Spécifications techniques

### Caractéristiques du code

| Aspect | Détails |
|--------|---------|
| **Lignes de code** | ~1400 lignes |
| **Fonctions** | 40+ fonctions modulaires |
| **Structures** | 3 structures principales (Client, Compte, Transaction) |
| **Variables globales** | Tableaux dynamiques (clients, comptes, transactions) |
| **Gestion mémoire** | malloc() / realloc() / free() |

### Architecture modulaire

#### Catégories de fonctions
- **Menus** : menu_principale(), Gestion_des_clients(), etc.
- **Utilitaires** : Validation, saisie sécurisée, formatage
- **Opérations CRUD** : Ajout, modification, suppression, recherche
- **Opérations financières** : Versement, retrait, virement
- **Transactions** : Création, affichage, historique
- **Persistance** : Sauvegarde, chargement, export
- **Admin** : Statistiques, consultation avancée

### Portabilité cross-platform

Le code est **compatible Windows et Linux** :
```c
#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif
```

### Bibliothèques utilisées
```c
#include <stdio.h>      // Entrées/sorties
#include <stdlib.h>     // malloc, free
#include <string.h>     // Manipulation de chaînes
#include <ctype.h>      // Validation de caractères
#include <time.h>       // Horodatage des transactions
```

### Gestion dynamique de la mémoire
```c
// Allocation initiale
clients = malloc(nb_clients * sizeof(Client));

// Ajout d'un élément
clients = realloc(clients, (nb_clients + 1) * sizeof(Client));

// Libération
free(clients);
```

**Aucune fuite mémoire** : Tous les malloc/realloc sont libérés avec free() à la fermeture.

---

## 📝 Scénario d'utilisation complet

### Exemple de session
```
1. Démarrage
   → Chargement silencieux de database/*.dat

2. Ajouter un client
   ID: 101, Nom: Alami, Prénom: Hassan
   Profession: Ingénieur, Tél: 0612345678

3. Créer un compte
   ID Compte: 2001, Client: 101
   Solde initial: 5000 DH, PIN: 1234
   → Transaction "Versement initial" créée automatiquement

4. Versement
   Compte: 2001, Montant: +3000 DH
   → Nouveau solde: 8000 DH

5. Retrait
   Compte: 2001, Montant: -700 DH (limite max)
   → Authentification PIN requise
   → Nouveau solde: 7300 DH

6. Créer un 2e compte
   ID Compte: 2002, Client: 102
   Solde: 2000 DH

7. Virement
   Source: 2001, Destination: 2002, Montant: 2000 DH
   → Authentification PIN compte 2001
   → 2 transactions créées
   → Solde 2001: 5300 DH
   → Solde 2002: 4000 DH

8. Consultation historique
   Compte: 2001, PIN: 1234
   → Affiche: Versement initial, Versement, Retrait, Virement

9. Mode Admin
   Menu principal → Saisir: #admin_bypass_system
   Code: benmsik_bank_admin_access
   → Accès aux statistiques globales

10. Fermeture
    → Sauvegarde auto des .dat
    → Export auto des .txt dans backdoor acces/
```

---

## 📚 Notes importantes

### Bonnes pratiques implémentées

✅ **Validation stricte** à chaque entrée utilisateur  
✅ **Messages d'erreur explicites** pour guider l'utilisateur  
✅ **Confirmations** pour les opérations critiques (suppressions)  
✅ **Masquage du PIN** (affichage ****)  
✅ **Auto-logging** de toutes les opérations  
✅ **Pas de fuites mémoire** (vérification avec valgrind)  
✅ **Code commenté** pour faciliter la maintenance  
✅ **Architecture modulaire** (40+ fonctions)  


---

## 👥 Équipe de développement

- **Hamza Bordo**
- **Ayoub Bizzari**
- **Taha Mahboub**

**Année Universitaire** : 2025-2026  
**Module** : Programmation C  
**Professeur** : Sanaa EL FILALI  

---


## 🎓 Conclusion

Ce projet démontre une **maîtrise complète** des concepts fondamentaux du langage C :

✅ Structures de données complexes  
✅ Gestion dynamique de la mémoire  
✅ Persistance multicouche (binaire + texte)  
✅ Architecture modulaire et maintenable  
✅ Sécurité et validations robustes  
✅ Fonctionnalités avancées (admin, statistiques, auto-logging)  

---

**Banque Ben M'sik** - Système de Gestion Bancaire en C  
*"Un projet académique qui simule un vrai système bancaire"* 🏦
