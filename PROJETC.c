#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define DB_PATH "database/"

typedef struct {
    int jour;
    int mois;
    int annee;
} Date;

typedef struct {
    int id_client;
    char nom[50];
    char prenom[50];
    char profession[50];
    char num_tel[15];
} Client;

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

Client clients[200];
Compte comptes[400];
Transaction transactions[5000];
int nb_clients = 0;
int nb_comptes = 0;
int nb_transactions = 0;
int next_transaction_id = 1;

// Function declarations
void menu_principale();
void Gestion_des_clients();
void Gestion_des_comptes();
void Gestion_des_operation();
void menu_admin();
int authentifier_admin();

// OPTION 2: Consolidated validation functions
int valider_chaine(char *s, int min_len, int max_len, int type) {
    // type: 0=alpha only, 1=digits only, 2=date validation
    int len = (int)strlen(s);
    if (len < min_len || len > max_len) return 0;

    for (int i = 0; i < len; i++) {
        if (type == 0 && isdigit((unsigned char)s[i])) return 0;
        if (type == 1 && !isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int valider_date(int jour, int mois, int annee) {
    return (jour >= 1 && jour <= 31 && mois >= 1 && mois <= 12 &&
            annee >= 1900 && annee <= 2100);
}

#define valider_nom(n) valider_chaine(n, 1, 50, 0)
#define valider_telephone(t) valider_chaine(t, 10, 14, 1)

int telephone_existe(char tel[], int exclude_id) {
    for (int i = 0; i < nb_clients; i++) {
        if (clients[i].id_client != exclude_id && strcmp(clients[i].num_tel, tel) == 0)
            return 1;
    }
    return 0;
}

// OPTION 6: Error handling macro
#define ERREUR_RETOUR(msg, menu_func) \
    do { printf(" %s ❌\n", msg); menu_func(); return; } while(0)

int verifier_pin(int id_compte) {
    int idx = -1;
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_compte == id_compte) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return 0;

    char pin[5];
    printf("\nEntrez le code PIN (4 chiffres): ");
    scanf("%4s", pin);

    if (strcmp(pin, comptes[idx].pin) == 0) {
        printf(" PIN correct ✅\n");
        return 1;
    }
    printf(" PIN incorrect ❌\n");
    return 0;
}

int chercherClientParId(int id) {
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].id_client == id) return i;
    return -1;
}

int chercherClientParNom(char nom[]) {
    for (int i = 0; i < nb_clients; i++)
        if (strcmp(clients[i].nom, nom) == 0) return i;
    return -1;
}

int compter_comptes_client(int id_client) {
    int count = 0;
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_client == id_client) count++;
    return count;
}

void supprimer_client_par_index(int index) {
    for (int j = index; j < nb_clients - 1; j++)
        clients[j] = clients[j + 1];
    nb_clients--;
}

int chercherCompteParId(int id) {
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_compte == id) return i;
    return -1;
}

void supprimer_compte_par_index(int index) {
    for (int j = index; j < nb_comptes - 1; j++)
        comptes[j] = comptes[j + 1];
    nb_comptes--;
}

void Ajoute_un_client() {
    Client c;
    printf("\n--- AJOUTER UN CLIENT ---\n");

    if (nb_clients >= 200)
        ERREUR_RETOUR("Nombre maximum de clients atteint", Gestion_des_clients);

    printf("Id client: ");
    scanf("%d", &c.id_client);

    if (chercherClientParId(c.id_client) != -1)
        ERREUR_RETOUR("Client existe déjà", Gestion_des_clients);

    do {
        printf("Nom: ");
        scanf("%s", c.nom);
        if (!valider_nom(c.nom)) printf(" Nom invalide (pas de chiffres) ❌\n");
    } while (!valider_nom(c.nom));

    do {
        printf("Prenom: ");
        scanf("%s", c.prenom);
        if (!valider_nom(c.prenom)) printf(" Prenom invalide (pas de chiffres) ❌\n");
    } while (!valider_nom(c.prenom));

    printf("Profession: ");
    scanf("%s", c.profession);

    do {
        printf("Telephone (10-14 chiffres): ");
        scanf("%s", c.num_tel);
        if (!valider_telephone(c.num_tel)) {
            printf(" Telephone invalide ❌\n");
        } else if (telephone_existe(c.num_tel, -1)) {
            printf(" Ce numero existe déjà ❌\n");
        } else {
            break;
        }
    } while (1);

    clients[nb_clients++] = c;
    printf(" Client ajouté avec succès ✅\n");
    Gestion_des_clients();
}

void Modifications() {
    int id, choix;
    printf("\n--- MODIFIER UN CLIENT ---\n");

    printf("Entrer Id client: ");
    scanf("%d", &id);

    int i = chercherClientParId(id);
    if (i == -1) ERREUR_RETOUR("Client introuvable", Gestion_des_clients);

    printf("\nClient actuel: %s %s\n", clients[i].nom, clients[i].prenom);
    printf("1. Nom\n2. Prenom\n3. Profession\n4. Telephone\n");
    printf("Votre choix: ");
    scanf("%d", &choix);

    char temp[50];
    switch (choix) {
        case 1:
            do {
                printf("Nouveau nom: ");
                scanf("%s", clients[i].nom);
                if (!valider_nom(clients[i].nom)) printf(" Nom invalide ❌\n");
            } while (!valider_nom(clients[i].nom));
            break;
        case 2:
            do {
                printf("Nouveau prenom: ");
                scanf("%s", clients[i].prenom);
                if (!valider_nom(clients[i].prenom)) printf(" Prenom invalide ❌\n");
            } while (!valider_nom(clients[i].prenom));
            break;
        case 3:
            printf("Nouvelle profession: ");
            scanf("%s", clients[i].profession);
            break;
        case 4:
            do {
                printf("Nouveau telephone: ");
                scanf("%s", temp);
                if (!valider_telephone(temp)) {
                    printf(" Telephone invalide ❌\n");
                } else if (telephone_existe(temp, id)) {
                    printf(" Ce numero existe déjà ❌\n");
                } else {
                    strcpy(clients[i].num_tel, temp);
                    break;
                }
            } while (1);
            break;
        default:
            ERREUR_RETOUR("Choix invalide", Gestion_des_clients);
    }

    printf(" Modification réussie ✅\n");
    Gestion_des_clients();
}

void Suppression() {
    int id;
    char rep;
    printf("\n--- SUPPRIMER UN CLIENT ---\n");

    printf("Entrer Id client: ");
    scanf("%d", &id);

    int i = chercherClientParId(id);
    if (i == -1) ERREUR_RETOUR("Client introuvable", Gestion_des_clients);

    int nb_comptes_cl = compter_comptes_client(id);
    if (nb_comptes_cl > 0) {
        printf(" Ce client possède %d compte(s)\n", nb_comptes_cl);
        printf("Voulez-vous vraiment supprimer ce client (O/N): ");
        scanf(" %c", &rep);
        if (rep != 'O' && rep != 'o') {
            printf(" Suppression annulée\n");
            Gestion_des_clients();
            return;
        }
    }

    printf("Client: %s %s\n", clients[i].nom, clients[i].prenom);
    printf("Confirmer suppression (O/N): ");
    scanf(" %c", &rep);

    if (rep != 'O' && rep != 'o') {
        printf(" Suppression annulée\n");
        Gestion_des_clients();
        return;
    }

    supprimer_client_par_index(i);
    printf(" Client supprimé ✅\n");
    Gestion_des_clients();
}

void Recherche() {
    int choix;
    printf("\n--- RECHERCHER UN CLIENT ---\n");
    printf("1. Recherche par ID\n2. Recherche par Nom\n");
    printf("Votre choix: ");
    scanf("%d", &choix);

    if (choix == 1) {
        int id;
        printf("Id client: ");
        scanf("%d", &id);
        int i = chercherClientParId(id);
        if (i != -1) {
            printf("\n Client trouvé ✅\n");
            printf("ID: %d | %s %s | %s | Tel: %s\n",
                   clients[i].id_client, clients[i].nom, clients[i].prenom,
                   clients[i].profession, clients[i].num_tel);
            printf("Nombre de comptes: %d\n", compter_comptes_client(id));
        } else {
            printf(" Client introuvable ❌\n");
        }
    } else if (choix == 2) {
        char nom[50];
        printf("Nom: ");
        scanf("%s", nom);
        int i = chercherClientParNom(nom);
        if (i != -1) {
            printf("\n Client trouvé ✅\n");
            printf("ID: %d | %s %s | %s | Tel: %s\n",
                   clients[i].id_client, clients[i].nom, clients[i].prenom,
                   clients[i].profession, clients[i].num_tel);
            printf("Nombre de comptes: %d\n", compter_comptes_client(clients[i].id_client));
        } else {
            printf(" Client introuvable ❌\n");
        }
    } else {
        printf(" Choix invalide ❌\n");
    }
    Gestion_des_clients();
}

void Nouveau_compte() {
    Compte c;
    printf("\n--- CREER UN COMPTE ---\n");

    if (nb_comptes >= 400)
        ERREUR_RETOUR("Nombre maximum de comptes atteint", Gestion_des_comptes);

    printf("Id compte: ");
    scanf("%d", &c.id_compte);

    if (chercherCompteParId(c.id_compte) != -1)
        ERREUR_RETOUR("Compte existe déjà", Gestion_des_comptes);

    printf("Id client: ");
    scanf("%d", &c.id_client);

    int idx = chercherClientParId(c.id_client);
    if (idx == -1) ERREUR_RETOUR("Client inexistant", Gestion_des_comptes);

    printf("\nClient: %s %s\n", clients[idx].nom, clients[idx].prenom);

    printf("Solde initial (min 1000 DH): ");
    scanf("%f", &c.solde);

    if (c.solde < 1000) ERREUR_RETOUR("Solde minimum 1000 DH", Gestion_des_comptes);

    do {
        printf("Date d'ouverture (j m a): ");
        scanf("%d %d %d", &c.date_ouverture.jour, &c.date_ouverture.mois, &c.date_ouverture.annee);
        if (!valider_date(c.date_ouverture.jour, c.date_ouverture.mois, c.date_ouverture.annee)) {
            printf(" Date invalide ❌\n");
        } else break;
    } while (1);

    do {
        printf("Definir un code PIN (4 chiffres): ");
        scanf("%4s", c.pin);

        if (strlen(c.pin) != 4 || !valider_chaine(c.pin, 4, 4, 1)) {
            printf(" Le PIN doit contenir 4 chiffres ❌\n");
            continue;
        }

        char confirm[5];
        printf("Confirmer le PIN: ");
        scanf("%4s", confirm);

        if (strcmp(c.pin, confirm) == 0) break;
        else printf(" Les PINs ne correspondent pas ❌\n");
    } while (1);

    comptes[nb_comptes++] = c;
    printf(" Compte créé avec succès ✅\n");

    // Add opening transaction
    if (nb_transactions < 5000) {
        transactions[nb_transactions].id_transaction = next_transaction_id++;
        transactions[nb_transactions].id_compte = c.id_compte;
        strcpy(transactions[nb_transactions].type, "Ouverture");
        transactions[nb_transactions].montant = c.solde;
        transactions[nb_transactions].compte_destination = -1;
        nb_transactions++;
    }
    Gestion_des_comptes();
}

void consultation() {
    int id;
    printf("\n--- CONSULTATION ---\n");

    printf("Id compte: ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) ERREUR_RETOUR("Compte introuvable", Gestion_des_comptes);

    if (!verifier_pin(id)) {
        Gestion_des_comptes();
        return;
    }

    int idx = chercherClientParId(comptes[i].id_client);
    printf("\n Informations du compte:\n");
    printf("======================================\n");
    printf("Compte: %d\n", comptes[i].id_compte);
    printf("Client: %s %s (ID: %d)\n",
           idx != -1 ? clients[idx].nom : "Inconnu",
           idx != -1 ? clients[idx].prenom : "",
           comptes[i].id_client);
    printf("Solde: %.2f DH\n", comptes[i].solde);
    printf("Date ouverture: %d/%d/%d\n",
           comptes[i].date_ouverture.jour,
           comptes[i].date_ouverture.mois,
           comptes[i].date_ouverture.annee);
    printf("======================================\n");

    Gestion_des_comptes();
}

void fermeture_du_compte() {
    char rep;
    printf("\n--- FERMER UN COMPTE ---\n");

    int id;
    printf("Id compte: ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) ERREUR_RETOUR("Compte introuvable", Gestion_des_comptes);

    if (!verifier_pin(id)) {
        printf(" Fermeture annulée ❌\n");
        Gestion_des_comptes();
        return;
    }

    printf("\nCompte %d | Solde: %.2f DH\n", comptes[i].id_compte, comptes[i].solde);
    printf("Confirmer suppression (O/N): ");
    scanf(" %c", &rep);

    if (rep != 'O' && rep != 'o') {
        printf(" Suppression annulée\n");
        Gestion_des_comptes();
        return;
    }

    supprimer_compte_par_index(i);
    printf(" Compte supprimé ✅\n");
    Gestion_des_comptes();
}

void ajouter_transaction(int id_compte, char type[], float montant, int compte_dest) {
    if (nb_transactions >= 5000) return;

    transactions[nb_transactions].id_transaction = next_transaction_id++;
    transactions[nb_transactions].id_compte = id_compte;
    strcpy(transactions[nb_transactions].type, type);
    transactions[nb_transactions].montant = montant;
    transactions[nb_transactions].compte_destination = compte_dest;
    nb_transactions++;
}

void Retrait() {
    int id;
    float m;

    printf("\n--- RETRAIT ---\n");
    printf("Id compte: ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) ERREUR_RETOUR("Compte introuvable", Gestion_des_operation);

    if (!verifier_pin(id)) {
        Gestion_des_operation();
        return;
    }

    printf("\nSolde actuel: %.2f DH\n", comptes[i].solde);
    printf("Montant a retirer: ");
    scanf("%f", &m);

    if (m <= 0) ERREUR_RETOUR("Montant invalide", Gestion_des_operation);
    if (m > 700) ERREUR_RETOUR("Limite de retrait: 700 DH maximum", Gestion_des_operation);
    if (comptes[i].solde < m) ERREUR_RETOUR("Solde insuffisant", Gestion_des_operation);

    comptes[i].solde -= m;
    ajouter_transaction(id, "Retrait", m, -1);

    printf(" Retrait effectué ✅\n");
    printf("Nouveau solde: %.2f DH\n", comptes[i].solde);
    Gestion_des_operation();
}

void Virement() {
    int c1, c2;
    float m;

    printf("\n--- VIREMENT ---\n");

    printf("Compte source: ");
    scanf("%d", &c1);

    int i1 = chercherCompteParId(c1);
    if (i1 == -1) ERREUR_RETOUR("Compte source introuvable", Gestion_des_operation);

    if (!verifier_pin(c1)) {
        Gestion_des_operation();
        return;
    }

    printf("Compte destination: ");
    scanf("%d", &c2);

    int i2 = chercherCompteParId(c2);
    if (i2 == -1) ERREUR_RETOUR("Compte destination introuvable", Gestion_des_operation);
    if (c1 == c2) ERREUR_RETOUR("Les comptes doivent être différents", Gestion_des_operation);

    printf("Solde compte source: %.2f DH\n", comptes[i1].solde);
    printf("Montant a virer: ");
    scanf("%f", &m);

    if (m <= 0) ERREUR_RETOUR("Montant invalide", Gestion_des_operation);
    if (comptes[i1].solde < m) ERREUR_RETOUR("Solde insuffisant", Gestion_des_operation);

    comptes[i1].solde -= m;
    comptes[i2].solde += m;

    ajouter_transaction(c1, "Virement", m, c2);
    ajouter_transaction(c2, "Virement", m, c1);

    printf(" Virement réussi ✅\n");
    printf("Nouveau solde source: %.2f DH\n", comptes[i1].solde);
    printf("Nouveau solde destination: %.2f DH\n", comptes[i2].solde);
    Gestion_des_operation();
}

// OPTION 5: Simplified history display (inline, no separate function)
void afficher_historique() {
    int id;
    printf("\n--- HISTORIQUE DES TRANSACTIONS ---\n");

    printf("Id compte: ");
    scanf("%d", &id);

    if (chercherCompteParId(id) == -1)
        ERREUR_RETOUR("Compte introuvable", Gestion_des_operation);

    if (!verifier_pin(id)) {
        Gestion_des_operation();
        return;
    }

    printf("\n======================================\n");
    printf("  HISTORIQUE DU COMPTE %d\n", id);
    printf("======================================\n");

    int found = 0;
    for (int i = 0; i < nb_transactions; i++) {
        if (transactions[i].id_compte == id) {
            printf("\n[Transaction #%d]\n", transactions[i].id_transaction);
            printf("Type: %s | Montant: %.2f DH", transactions[i].type, transactions[i].montant);
            if (transactions[i].compte_destination != -1)
                printf(" | Compte lié: %d", transactions[i].compte_destination);
            printf("\n--------------------------------------");
            found = 1;
        }
    }

    if (!found) printf("\n Aucune transaction pour ce compte\n");
    printf("\n======================================\n");
    Gestion_des_operation();
}

void sauvegarder_donnees() {
    FILE *f;

    f = fopen(DB_PATH "clients.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_clients, sizeof(int), 1, f);
        fwrite(clients, sizeof(Client), nb_clients, f);
        fclose(f);
    }

    f = fopen(DB_PATH "comptes.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_comptes, sizeof(int), 1, f);
        fwrite(comptes, sizeof(Compte), nb_comptes, f);
        fclose(f);
    }

    f = fopen(DB_PATH "transactions.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_transactions, sizeof(int), 1, f);
        fwrite(&next_transaction_id, sizeof(int), 1, f);
        fwrite(transactions, sizeof(Transaction), nb_transactions, f);
        fclose(f);
    }
}


void charger_donnees() {
    FILE *f;
    int n;

    f = fopen(DB_PATH "clients.dat", "rb");
    if (f != NULL) {
        fread(&n, sizeof(int), 1, f);
        nb_clients = (n > 200) ? 200 : n;
        fread(clients, sizeof(Client), nb_clients, f);
        fclose(f);
    } else {
        nb_clients = 0;
    }

    f = fopen(DB_PATH "comptes.dat", "rb");
    if (f != NULL) {
        fread(&n, sizeof(int), 1, f);
        nb_comptes = (n > 400) ? 400 : n;
        fread(comptes, sizeof(Compte), nb_comptes, f);
        fclose(f);
    } else {
        nb_comptes = 0;
    }

    f = fopen(DB_PATH "transactions.dat", "rb");
    if (f != NULL) {
        fread(&n, sizeof(int), 1, f);
        nb_transactions = (n > 5000) ? 5000 : n;
        fread(&next_transaction_id, sizeof(int), 1, f);
        fread(transactions, sizeof(Transaction), nb_transactions, f);
        fclose(f);
    } else {
        nb_transactions = 0;
        next_transaction_id = 1;
    }
}


int authentifier_admin() {
    char code[100];
    printf("\n=======================================\n");
    printf("||      ACCES ADMINISTRATEUR          ||\n");
    printf("=======================================\n");
    printf("Entrez le code d'acces admin: ");
    scanf("%s", code);

    if (strcmp(code, "benmsik_bank_admin_access") == 0) {
        printf("\n Acces autorisé ✅\n");
        return 1;
    }
    printf("\n Code d'acces invalide ❌\n");
    return 0;
}

// ADMIN SECTION - Kept only: afficher_tous_clients_detaille, afficher_tous_comptes_detaille,
// afficher_toutes_transactions_detaille, afficher_client_complet

void afficher_tous_clients_detaille() {
    printf("\n=======================================\n");
    printf("||    TOUS LES CLIENTS (DETAILLE)     ||\n");
    printf("=======================================\n");

    if (nb_clients == 0) {
        printf("\n Aucun client enregistré\n");
        menu_admin();
        return;
    }

    for (int i = 0; i < nb_clients; i++) {
        printf("\n--- Client #%d ---\n", i + 1);
        printf("ID: %d | %s %s | %s | Tel: %s\n",
               clients[i].id_client, clients[i].nom, clients[i].prenom,
               clients[i].profession, clients[i].num_tel);
        printf("Nombre de comptes: %d\n", compter_comptes_client(clients[i].id_client));
        printf("--------------------------------------\n");
    }
    menu_admin();
}

void afficher_tous_comptes_detaille() {
    printf("\n=======================================\n");
    printf("||    TOUS LES COMPTES (DETAILLE)     ||\n");
    printf("=======================================\n");

    if (nb_comptes == 0) {
        printf("\n Aucun compte enregistré\n");
        menu_admin();
        return;
    }

    for (int i = 0; i < nb_comptes; i++) {
        int idx = chercherClientParId(comptes[i].id_client);
        printf("\n--- Compte #%d ---\n", i + 1);
        printf("ID: %d | Client: %s %s | Solde: %.2f DH\n",
               comptes[i].id_compte,
               idx != -1 ? clients[idx].nom : "Inconnu",
               idx != -1 ? clients[idx].prenom : "",
               comptes[i].solde);
        printf("Date: %d/%d/%d | PIN: %s\n",
               comptes[i].date_ouverture.jour,
               comptes[i].date_ouverture.mois,
               comptes[i].date_ouverture.annee,
               comptes[i].pin);
        printf("--------------------------------------\n");
    }
    menu_admin();
}

void afficher_toutes_transactions_detaille() {
    printf("\n=======================================\n");
    printf("|| TOUTES LES TRANSACTIONS (DETAILLE) ||\n");
    printf("=======================================\n");

    if (nb_transactions == 0) {
        printf("\n Aucune transaction enregistrée\n");
        menu_admin();
        return;
    }

    for (int i = 0; i < nb_transactions; i++) {
        printf("\n[#%d] Compte: %d | %s | %.2f DH",
               transactions[i].id_transaction,
               transactions[i].id_compte,
               transactions[i].type,
               transactions[i].montant);
        if (transactions[i].compte_destination != -1)
            printf(" → %d", transactions[i].compte_destination);
        printf("\n");
    }
    menu_admin();
}



void menu_admin() {
    int choix;
    do {
        printf("\n=======================================\n");
        printf("||      MENU ADMINISTRATEUR          ||\n");
        printf("=======================================\n");
        printf("||  1. Voir tous les clients        ||\n");
        printf("||  2. Voir tous les comptes        ||\n");
        printf("||  3. Voir toutes les transactions ||\n");
        printf("||  4. Retour au menu principal     ||\n");
        printf("=======================================\n");
        printf("Votre choix: ");
        scanf("%d", &choix);
    } while (choix < 1 || choix > 5);

    switch(choix) {
        case 1: afficher_tous_clients_detaille(); break;
        case 2: afficher_tous_comptes_detaille(); break;
        case 3: afficher_toutes_transactions_detaille(); break;
        case 4: menu_principale(); break;
    }
}

void Gestion_des_clients() {
    int b;
    do {
        printf("\n**************************************\n");
        printf("*       GESTION DES CLIENTS          *\n");
        printf("**************************************\n");
        printf("*   1. Ajouter un client             *\n");
        printf("*   2. Modifier un client            *\n");
        printf("*   3. Supprimer un client           *\n");
        printf("*   4. Rechercher un client          *\n");
        printf("*   5. Retour au menu principal      *\n");
        printf("**************************************\n");
        printf("Votre choix: ");
        scanf("%d", &b);
    } while (b < 1 || b > 5);

    switch(b) {
        case 1: Ajoute_un_client(); break;
        case 2: Modifications(); break;
        case 3: Suppression(); break;
        case 4: Recherche(); break;
        case 5: menu_principale(); break;
    }
}

void Gestion_des_comptes() {
    int b;
    do {
        printf("\n**************************************\n");
        printf("*       GESTION DES COMPTES          *\n");
        printf("**************************************\n");
        printf("*   1. Nouveau compte                *\n");
        printf("*   2. Consultation                  *\n");
        printf("*   3. Fermeture du compte           *\n");
        printf("*   4. Retour au menu principal      *\n");
        printf("**************************************\n");
        printf("Votre choix: ");
        scanf("%d", &b);
    } while (b < 1 || b > 4);

    switch(b) {
        case 1: Nouveau_compte(); break;
        case 2: consultation(); break;
        case 3: fermeture_du_compte(); break;
        case 4: menu_principale(); break;
    }
}

void Gestion_des_operation() {
    int b;
    do {
        printf("\n**************************************\n");
        printf("*      GESTION DES OPERATIONS        *\n");
        printf("**************************************\n");
        printf("*   1. Retrait                       *\n");
        printf("*   2. Virement                      *\n");
        printf("*   3. Historique des transactions   *\n");
        printf("*   4. Retour au menu principal      *\n");
        printf("**************************************\n");
        printf("Votre choix: ");
        scanf("%d", &b);
    } while (b < 1 || b > 4);

    switch(b) {
        case 1: Retrait(); break;
        case 2: Virement(); break;
        case 3: afficher_historique(); break;
        case 4: menu_principale(); break;
    }
}

void menu_principale() {
    int a;
    char input[100];
    do {
        printf("\n======================================\n");
        printf("||    SYSTEME DE GESTION BANCAIRE   ||\n");
        printf("======================================\n");
        printf("||          MENU PRINCIPAL          ||\n");
        printf("======================================\n");
        printf("||  1. Gestion des clients          ||\n");
        printf("||  2. Gestion des comptes          ||\n");
        printf("||  3. Gestion des operations       ||\n");
        printf("||  4. Quitter                      ||\n");
        printf("======================================\n");
        printf("Votre choix: ");

        scanf("%s", input);

        if (strcmp(input, "#admin_bypass_system") == 0) {
            if (authentifier_admin()) {
                menu_admin();
                continue;
            } else {
                continue;
            }
        }

        if (input[0] >= '0' && input[0] <= '9' && input[1] == '\0') {
            a = input[0] - '0';
        } else {
            printf(" Choix invalide ❌\n");
            continue;
        }
    } while (a < 1 || a > 4);

    switch(a) {
        case 1: Gestion_des_clients(); break;
        case 2: Gestion_des_comptes(); break;
        case 3: Gestion_des_operation(); break;
        case 4:
            printf("\n Au revoir! ✅\n");
            return;
    }
}

void run_application() {
    printf("\n=======================================\n");
    printf("     Banque Ben M'sik                  \n");
    printf("=======================================\n");

    charger_donnees();
    menu_principale();
    sauvegarder_donnees();
}

int main() {
    run_application();
    return 0;
}
