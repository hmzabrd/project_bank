#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <direct.h>
#include <conio.h>
#include <windows.h>

#define DB_PATH "database/"

#define SOLDE_MIN 1000.0
#define RETRAIT_MAX 700.0
#define VIREMENT_MAX 5000.0
#define PIN_LENGTH 4

#define MAX_CLIENTS 1000
#define MAX_COMPTES 1000
#define MAX_TRANSACTIONS 10000

#define COLOR_RESET   "\x1b[0m"
#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_BLUE    "\x1b[34m"
#define COLOR_MAGENTA "\x1b[35m"
#define COLOR_CYAN    "\x1b[36m"


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
    int id_compte;  // Will be 9-digit RIB-like number
    int id_client;
    float solde;
    Date date_ouverture;
    char pin[PIN_LENGTH + 1];
} Compte;

typedef struct {
    int id_transaction;
    int id_compte;
    char type[20];
    float montant;
    int compte_destination;
} Transaction;

Client clients[MAX_CLIENTS];
Compte comptes[MAX_COMPTES];
Transaction transactions[MAX_TRANSACTIONS];

int nb_clients = 0;
int nb_comptes = 0;
int nb_transactions = 0;

// Auto-increment counters
int next_client_id = 1;
int next_compte_rib = 1;  // Will generate 000000001, 000000002, etc.
int next_transaction_id = 1;

void menu_principale();
void Gestion_des_clients();
void Gestion_des_comptes();
void Gestion_des_operation();
void menu_admin();
void journaliser_evenement(const char *message);
int authentifier_admin();
void sauvegarder_donnees();
void charger_donnees();
void clear_screen();
void pause_ecran();
int menu_avec_fleches(char *titre, char *options[], int nb_options);

void afficher_chargement(const char *message) {
    printf("\n%s", message);
    for(int i = 0; i < 3; i++) {
        printf(".");
        fflush(stdout);
        #ifdef _WIN32
        Sleep(300);  // Add #include <windows.h> at top
        #else
        usleep(300000);
        #endif
    }
    printf(" " COLOR_GREEN "OK!\n" COLOR_RESET);
    #ifdef _WIN32
        Sleep(300);  // Add #include <windows.h> at top
        #else
        usleep(300000);
        #endif
}

void pause_ecran() {
    printf("\n" COLOR_YELLOW "Appuyez sur Entree pour continuer..." COLOR_RESET);
    while(getchar() != '\n');
    getchar();
}

void clear_screen() {
    system("cls");
}

int menu_avec_fleches(char *titre, char *options[], int nb_options) {
    int selection = 0;
    int key;

    while(1) {
        clear_screen();

        // Display title
        printf("%s", titre);

        printf("\n" COLOR_YELLOW "Utilisez les fleches HAUT/BAS pour naviguer\n");
        printf("Appuyez sur ENTREE pour selectionner\n" COLOR_RESET);

        // Display menu with highlighting
        for(int i = 0; i < nb_options; i++) {
            if(i == selection) {
                printf(COLOR_GREEN "  >> %s\n" COLOR_RESET, options[i]);
            } else {
                printf("     %s\n", options[i]);
            }
        }

        key = _getch();

        // Check for admin access key
        if(key == '#') {
            return -999; // Special code to trigger admin access
        }

        // Handle arrow keys
        if(key == 224 || key == 0) {
            key = _getch();
            if(key == 72) { // Up arrow
                selection--;
                if(selection < 0) selection = nb_options - 1;
            }
            else if(key == 80) { // Down arrow
                selection++;
                if(selection >= nb_options) selection = 0;
            }
        }
        else if(key == 13) { // Enter key
            return selection + 1;
        }
    }
}

int valider_date(int jour, int mois, int annee) {
    if (annee < 1900 || annee > 2100) return 0;
    if (mois < 1 || mois > 12) return 0;

    int jours_par_mois[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    if (mois == 2) {
        int est_bissextile = (annee % 4 == 0 && annee % 100 != 0) || (annee % 400 == 0);
        if (est_bissextile) {
            jours_par_mois[1] = 29;
        }
    }

    return (jour >= 1 && jour <= jours_par_mois[mois - 1]);
}

int valider_chaine(char *s, int min_len, int max_len, int type) {
    int len = (int)strlen(s);

    if (len < min_len || len > max_len) return 0;

    for (int i = 0; i < len; i++) {
        if (type == 0 && isdigit((unsigned char)s[i])) {
            return 0;
        }
        if (type == 1 && !isdigit((unsigned char)s[i])) {
            return 0;
        }
    }
    return 1;
}

#define valider_nom(n) valider_chaine(n, 1, 49, 0)
#define valider_telephone(t) valider_chaine(t, 10, 14, 1)

int telephone_existe(char tel[], int exclude_id) {
    for (int i = 0; i < nb_clients; i++) {
        if (clients[i].id_client != exclude_id && strcmp(clients[i].num_tel, tel) == 0) {
            return 1;
        }
    }
    return 0;
}

#define ERREUR_RETOUR(msg, menu_func) \
    do { \
        printf(COLOR_RED "%s\n" COLOR_RESET, msg); \
        pause_ecran(); \
        menu_func(); \
        return; \
    } while(0)

int verifier_pin(int id_compte) {
    int idx = -1;
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_compte == id_compte) {
            idx = i;
            break;
        }
    }

    if (idx == -1) return 0;

    char pin[PIN_LENGTH + 1];
    printf("\nEntrez le code PIN (%d chiffres): ", PIN_LENGTH);
    scanf("%4s", pin);

    if (strcmp(pin, comptes[idx].pin) == 0) {
        printf(COLOR_GREEN "PIN correct!\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_RED "PIN incorrect!\n" COLOR_RESET);
    return 0;
}

int chercherClientParId(int id) {
    for (int i = 0; i < nb_clients; i++) {
        if (clients[i].id_client == id) {
            return i;
        }
    }
    return -1;
}

int chercherClientParNom(char nom[]) {
    for (int i = 0; i < nb_clients; i++) {
        if (strcmp(clients[i].nom, nom) == 0) {
            return i;
        }
    }
    return -1;
}

int compter_comptes_client(int id_client) {
    int count = 0;
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_client == id_client) {
            count++;
        }
    }
    return count;
}

void supprimer_client_par_index(int index) {
    for (int j = index; j < nb_clients - 1; j++) {
        clients[j] = clients[j + 1];
    }
    nb_clients--;
}

int chercherCompteParId(int id) {
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_compte == id) {
            return i;
        }
    }
    return -1;
}

void supprimer_compte_par_index(int index) {
    for (int j = index; j < nb_comptes - 1; j++) {
        comptes[j] = comptes[j + 1];
    }
    nb_comptes--;
}

// ========================================
// FONCTIONS DE GESTION DES CLIENTS
// ========================================

void Ajoute_un_client() {
    Client c;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- AJOUTER UN CLIENT ---\n" COLOR_RESET);

    // Auto-increment ID - no user input
    if (nb_clients >= MAX_CLIENTS) {
        printf(COLOR_RED "Nombre maximal de clients atteint.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_clients();
        return;
    }
    c.id_client = next_client_id;

    do {
        printf("Nom: ");
        scanf("%49s", c.nom);
        if (!valider_nom(c.nom)) {
            printf(COLOR_RED "Nom invalide (pas de chiffres autorises)\n" COLOR_RESET);
        }
    } while (!valider_nom(c.nom));

    do {
        printf("Prenom: ");
        scanf("%49s", c.prenom);
        if (!valider_nom(c.prenom)) {
            printf(COLOR_RED "Prenom invalide (pas de chiffres autorises)\n" COLOR_RESET);
        }
    } while (!valider_nom(c.prenom));

    printf("Profession: ");
    scanf("%49s", c.profession);

    do {
        printf("Telephone (10-14 chiffres): ");
        scanf("%14s", c.num_tel);

        if (!valider_telephone(c.num_tel)) {
            printf(COLOR_RED "Telephone invalide (doit contenir que des chiffres)\n" COLOR_RESET);
        } else if (telephone_existe(c.num_tel, -1)) {
            printf(COLOR_RED "Ce numero de telephone existe deja!\n" COLOR_RESET);
        } else {
            break;
        }
    } while (1);

    clients[nb_clients] = c;
    nb_clients++;
    next_client_id++;  // Increment for next client

    printf(COLOR_GREEN "Client ajoute avec succes! ID: %d\n" COLOR_RESET, c.id_client);
    sprintf(log_msg, "Ajout client: ID=%d, Nom=%s, Prenom=%s, Tel=%s",
            c.id_client, c.nom, c.prenom, c.num_tel);
    journaliser_evenement(log_msg);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_clients();
}

void Modifications() {
    int id, choix;
    printf("\n" COLOR_CYAN "--- MODIFIER UN CLIENT ---\n" COLOR_RESET);

    printf("Entrez l'ID du client: ");
    scanf("%d", &id);

    int i = chercherClientParId(id);
    if (i == -1) {
        ERREUR_RETOUR("Client introuvable!", Gestion_des_clients);
    }

    printf("\nClient trouve:\n");
    printf("ID: %d | Nom: %s | Prenom: %s | Profession: %s | Tel: %s\n",
           clients[i].id_client, clients[i].nom, clients[i].prenom,
           clients[i].profession, clients[i].num_tel);

    do {
        printf("\n" COLOR_YELLOW "Que voulez-vous modifier?\n" COLOR_RESET);
        printf("1. Nom\n");
        printf("2. Prenom\n");
        printf("3. Profession\n");
        printf("4. Telephone\n");
        printf("5. Retour\n");
        printf("Choix: ");
        scanf("%d", &choix);
    } while (choix < 1 || choix > 5);

    switch(choix) {
        case 1:
            do {
                printf("Nouveau nom: ");
                scanf("%49s", clients[i].nom);
                if (!valider_nom(clients[i].nom)) {
                    printf(COLOR_RED "Nom invalide\n" COLOR_RESET);
                }
            } while (!valider_nom(clients[i].nom));
            break;
        case 2:
            do {
                printf("Nouveau prenom: ");
                scanf("%49s", clients[i].prenom);
                if (!valider_nom(clients[i].prenom)) {
                    printf(COLOR_RED "Prenom invalide\n" COLOR_RESET);
                }
            } while (!valider_nom(clients[i].prenom));
            break;
        case 3:
            printf("Nouvelle profession: ");
            scanf("%49s", clients[i].profession);
            break;
        case 4:
            do {
                printf("Nouveau telephone: ");
                scanf("%14s", clients[i].num_tel);
                if (!valider_telephone(clients[i].num_tel)) {
                    printf(COLOR_RED "Telephone invalide\n" COLOR_RESET);
                } else if (telephone_existe(clients[i].num_tel, clients[i].id_client)) {
                    printf(COLOR_RED "Ce numero existe deja!\n" COLOR_RESET);
                } else {
                    break;
                }
            } while (1);
            break;
        case 5:
            Gestion_des_clients();
            return;
    }

    printf(COLOR_GREEN "Modification reussie!\n" COLOR_RESET);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_clients();
}

void Suppression() {
    int id;
    char confirmation;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- SUPPRIMER UN CLIENT ---\n" COLOR_RESET);

    printf("Entrez l'ID du client: ");
    scanf("%d", &id);

    int i = chercherClientParId(id);
    if (i == -1) {
        ERREUR_RETOUR("Client introuvable!", Gestion_des_clients);
    }

    int nb_comptes_client = compter_comptes_client(id);
    if (nb_comptes_client > 0) {
        printf(COLOR_RED "Impossible! Ce client possede %d compte(s) actif(s).\n" COLOR_RESET, nb_comptes_client);
        printf(COLOR_YELLOW "Fermez d'abord tous les comptes du client.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_clients();
        return;
    }

    printf("\nClient a supprimer:\n");
    printf("ID: %d | Nom: %s | Prenom: %s\n",
           clients[i].id_client, clients[i].nom, clients[i].prenom);

    printf(COLOR_RED "\nEtes-vous sur de vouloir supprimer ce client? (O/N): " COLOR_RESET);
    scanf(" %c", &confirmation);

    if (confirmation == 'O' || confirmation == 'o') {
        supprimer_client_par_index(i);
        printf(COLOR_GREEN "Client supprime avec succes!\n" COLOR_RESET);
        sprintf(log_msg, "Suppression client: ID=%d, Nom=%s, Prenom=%s",
                id, clients[i].nom, clients[i].prenom);
        journaliser_evenement(log_msg);
        sauvegarder_donnees();
    } else {
        printf(COLOR_YELLOW "Suppression annulee.\n" COLOR_RESET);
    }

    pause_ecran();
    Gestion_des_clients();
}

void Recherche() {
    int choix, id;
    char nom[50];
    printf("\n" COLOR_CYAN "--- RECHERCHER UN CLIENT ---\n" COLOR_RESET);

    do {
        printf("1. Rechercher par ID\n");
        printf("2. Rechercher par nom\n");
        printf("3. Retour\n");
        printf("Choix: ");
        scanf("%d", &choix);
    } while (choix < 1 || choix > 3);

    if (choix == 3) {
        Gestion_des_clients();
        return;
    }

    int i = -1;
    if (choix == 1) {
        printf("ID du client: ");
        scanf("%d", &id);
        i = chercherClientParId(id);
    } else {
        printf("Nom du client: ");
        scanf("%49s", nom);
        i = chercherClientParNom(nom);
    }

    if (i == -1) {
        printf(COLOR_RED "Client introuvable!\n" COLOR_RESET);
    } else {
        printf("\n" COLOR_GREEN "Client trouve:\n" COLOR_RESET);
        printf("+------+----------------------+----------------------+----------------------+---------------+\n");
        printf("| %-4s | %-20s | %-20s | %-20s | %-13s |\n",
               "ID", "Nom", "Prenom", "Profession", "Telephone");
        printf("+------+----------------------+----------------------+----------------------+---------------+\n");
        printf("| %-4d | %-20s | %-20s | %-20s | %-13s |\n",
               clients[i].id_client, clients[i].nom, clients[i].prenom,
               clients[i].profession, clients[i].num_tel);
        printf("+------+----------------------+----------------------+----------------------+---------------+\n");
        printf("Nombre de comptes: %d\n", compter_comptes_client(clients[i].id_client));
    }

    pause_ecran();
    Gestion_des_clients();
}

// ========================================
// FONCTIONS DE GESTION DES COMPTES
// ========================================

void Nouveau_compte() {
    Compte c;
    int id_client;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- NOUVEAU COMPTE ---\n" COLOR_RESET);

    if (nb_comptes >= MAX_COMPTES) {
        printf(COLOR_RED "Nombre maximal de comptes atteint.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_comptes();
        return;
    }

    printf("ID du client: ");
    scanf("%d", &id_client);

    if (chercherClientParId(id_client) == -1) {
        ERREUR_RETOUR("Client inexistant!", Gestion_des_comptes);
    }

    // Generate RIB-like account number (9 digits)
    c.id_compte = next_compte_rib;
    c.id_client = id_client;

    do {
        printf("Solde initial (minimum %.2f DH): ", SOLDE_MIN);
        scanf("%f", &c.solde);
        if (c.solde < SOLDE_MIN) {
            printf(COLOR_RED "Le solde doit etre au moins %.2f DH!\n" COLOR_RESET, SOLDE_MIN);
        }
    } while (c.solde < SOLDE_MIN);

    do {
        printf("Date d'ouverture (JJ MM AAAA): ");
        scanf("%d %d %d", &c.date_ouverture.jour, &c.date_ouverture.mois, &c.date_ouverture.annee);
        if (!valider_date(c.date_ouverture.jour, c.date_ouverture.mois, c.date_ouverture.annee)) {
            printf(COLOR_RED "Date invalide!\n" COLOR_RESET);
        }
    } while (!valider_date(c.date_ouverture.jour, c.date_ouverture.mois, c.date_ouverture.annee));

    do {
        printf("Code PIN (%d chiffres): ", PIN_LENGTH);
        scanf("%4s", c.pin);
        if (!valider_chaine(c.pin, PIN_LENGTH, PIN_LENGTH, 1)) {
            printf(COLOR_RED "PIN invalide (doit contenir %d chiffres)!\n" COLOR_RESET, PIN_LENGTH);
        }
    } while (!valider_chaine(c.pin, PIN_LENGTH, PIN_LENGTH, 1));

    comptes[nb_comptes] = c;
    nb_comptes++;
    next_compte_rib++;  // Increment for next account

    printf(COLOR_GREEN "Compte cree avec succes! RIB: %09d\n" COLOR_RESET, c.id_compte);
    sprintf(log_msg, "Ouverture compte: RIB=%09d, ClientID=%d, SoldeInitial=%.2f",
            c.id_compte, c.id_client, c.solde);
    journaliser_evenement(log_msg);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_comptes();
}

void consultation() {
    int choix, id;
    printf("\n" COLOR_CYAN "--- CONSULTATION ---\n" COLOR_RESET);

    do {
        printf("1. Afficher tous les comptes\n");
        printf("2. Afficher un compte specifique\n");
        printf("3. Retour\n");
        printf("Choix: ");
        scanf("%d", &choix);
    } while (choix < 1 || choix > 3);

    if (choix == 3) {
        Gestion_des_comptes();
        return;
    }

    if (choix == 1) {
        if (nb_comptes == 0) {
            printf(COLOR_YELLOW "Aucun compte enregistre.\n" COLOR_RESET);
        } else {
            printf("\n+------------+----------+---------------+------------+\n");
            printf("| %-10s | %-8s | %-13s | %-10s |\n",
                   "RIB", "Client", "Solde (DH)", "Date");
            printf("+------------+----------+---------------+------------+\n");
            for (int i = 0; i < nb_comptes; i++) {
                printf("| %09d | %-8d | %13.2f | %02d/%02d/%d |\n",
                       comptes[i].id_compte,
                       comptes[i].id_client,
                       comptes[i].solde,
                       comptes[i].date_ouverture.jour,
                       comptes[i].date_ouverture.mois,
                       comptes[i].date_ouverture.annee);
            }
            printf("+------------+----------+---------------+------------+\n");
        }
    } else {
        printf("RIB du compte (9 chiffres): ");
        scanf("%d", &id);

        int i = chercherCompteParId(id);
        if (i == -1) {
            printf(COLOR_RED "Compte introuvable!\n" COLOR_RESET);
        } else {
            int idx_client = chercherClientParId(comptes[i].id_client);
            printf("\n" COLOR_GREEN "Informations du compte:\n" COLOR_RESET);
            printf("RIB: %09d\n", comptes[i].id_compte);
            printf("Client ID: %d\n", comptes[i].id_client);
            if (idx_client != -1) {
                printf("Titulaire: %s %s\n", clients[idx_client].nom, clients[idx_client].prenom);
            }
            printf("Solde: %.2f DH\n", comptes[i].solde);
            printf("Date ouverture: %02d/%02d/%d\n",
                   comptes[i].date_ouverture.jour,
                   comptes[i].date_ouverture.mois,
                   comptes[i].date_ouverture.annee);
        }
    }

    pause_ecran();
    Gestion_des_comptes();
}

void fermeture_du_compte() {
    int id;
    char confirmation;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- FERMETURE DU COMPTE ---\n" COLOR_RESET);

    printf("RIB du compte (9 chiffres): ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) {
        ERREUR_RETOUR("Compte introuvable!", Gestion_des_comptes);
    }

    /* Vérifier le PIN avant toute suppression de compte */
    if (!verifier_pin(id)) {
        printf(COLOR_YELLOW "Fermeture annulee (PIN incorrect).\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_comptes();
        return;
    }

    printf("\nCompte a fermer:\n");
    printf("RIB: %09d | Client: %d | Solde: %.2f DH\n",
           comptes[i].id_compte, comptes[i].id_client, comptes[i].solde);

    printf(COLOR_RED "\nEtes-vous sur de vouloir fermer ce compte? (O/N): " COLOR_RESET);
    scanf(" %c", &confirmation);

    if (confirmation == 'O' || confirmation == 'o') {
        sprintf(log_msg, "Fermeture compte: RIB=%09d, ClientID=%d, SoldeFinal=%.2f",
                comptes[i].id_compte, comptes[i].id_client, comptes[i].solde);
        journaliser_evenement(log_msg);
        supprimer_compte_par_index(i);
        printf(COLOR_GREEN "Compte ferme avec succes!\n" COLOR_RESET);
        sauvegarder_donnees();
    } else {
        printf(COLOR_YELLOW "Fermeture annulee.\n" COLOR_RESET);
    }

    pause_ecran();
    Gestion_des_comptes();
}

// ========================================
// FONCTIONS DE GESTION DES OPERATIONS
// ========================================

void Depot() {
    int id;
    float montant;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- DEPOT ---\n" COLOR_RESET);

    printf("RIB du compte: ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) {
        ERREUR_RETOUR("Compte introuvable!", Gestion_des_operation);
    }

    if (!verifier_pin(id)) {
        ERREUR_RETOUR("PIN incorrect!", Gestion_des_operation);
    }

    printf("Montant a deposer (DH): ");
    scanf("%f", &montant);

    if (montant <= 0) {
        ERREUR_RETOUR("Montant invalide!", Gestion_des_operation);
    }

    comptes[i].solde += montant;

    if (nb_transactions >= MAX_TRANSACTIONS) {
        printf(COLOR_RED "Nombre maximal de transactions atteint.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_operation();
        return;
    }
    Transaction t;
    t.id_transaction = next_transaction_id++;
    t.id_compte = id;
    strcpy(t.type, "Depot");
    t.montant = montant;
    t.compte_destination = -1;
    transactions[nb_transactions++] = t;

    printf(COLOR_GREEN "Depot reussi! Nouveau solde: %.2f DH\n" COLOR_RESET, comptes[i].solde);
    sprintf(log_msg, "Depot: RIB=%09d, Montant=%.2f, NouveauSolde=%.2f",
            id, montant, comptes[i].solde);
    journaliser_evenement(log_msg);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_operation();
}

void Retrait() {
    int id;
    float montant;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- RETRAIT ---\n" COLOR_RESET);

    printf("RIB du compte: ");
    scanf("%d", &id);

    int i = chercherCompteParId(id);
    if (i == -1) {
        ERREUR_RETOUR("Compte introuvable!", Gestion_des_operation);
    }

    if (!verifier_pin(id)) {
        ERREUR_RETOUR("PIN incorrect!", Gestion_des_operation);
    }

    printf("Montant a retirer (max %.2f DH): ", RETRAIT_MAX);
    scanf("%f", &montant);

    if (montant <= 0 || montant > RETRAIT_MAX) {
        ERREUR_RETOUR("Montant invalide ou depasse la limite!", Gestion_des_operation);
    }

    if (comptes[i].solde < montant) {
        ERREUR_RETOUR("Solde insuffisant!", Gestion_des_operation);
    }

    comptes[i].solde -= montant;

    if (nb_transactions >= MAX_TRANSACTIONS) {
        printf(COLOR_RED "Nombre maximal de transactions atteint.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_operation();
        return;
    }
    Transaction t;
    t.id_transaction = next_transaction_id++;
    t.id_compte = id;
    strcpy(t.type, "Retrait");
    t.montant = montant;
    t.compte_destination = -1;
    transactions[nb_transactions++] = t;

    printf(COLOR_GREEN "Retrait reussi! Nouveau solde: %.2f DH\n" COLOR_RESET, comptes[i].solde);
    sprintf(log_msg, "Retrait: RIB=%09d, Montant=%.2f, NouveauSolde=%.2f",
            id, montant, comptes[i].solde);
    journaliser_evenement(log_msg);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_operation();
}

void Virement() {
    int id_source, id_dest;
    float montant;
    char log_msg[256];
    printf("\n" COLOR_CYAN "--- VIREMENT ---\n" COLOR_RESET);

    printf("RIB du compte source: ");
    scanf("%d", &id_source);

    int i_source = chercherCompteParId(id_source);
    if (i_source == -1) {
        ERREUR_RETOUR("Compte source introuvable!", Gestion_des_operation);
    }

    if (!verifier_pin(id_source)) {
        ERREUR_RETOUR("PIN incorrect!", Gestion_des_operation);
    }

    printf("RIB du compte destinataire: ");
    scanf("%d", &id_dest);

    int i_dest = chercherCompteParId(id_dest);
    if (i_dest == -1) {
        ERREUR_RETOUR("Compte destinataire introuvable!", Gestion_des_operation);
    }

    if (id_source == id_dest) {
        ERREUR_RETOUR("Les comptes source et destination doivent etre differents!", Gestion_des_operation);
    }

    printf("Montant a virer (max %.2f DH): ", VIREMENT_MAX);
    scanf("%f", &montant);

    if (montant <= 0 || montant > VIREMENT_MAX) {
        ERREUR_RETOUR("Montant invalide ou depasse la limite!", Gestion_des_operation);
    }

    if (comptes[i_source].solde < montant) {
        ERREUR_RETOUR("Solde insuffisant!", Gestion_des_operation);
    }

    comptes[i_source].solde -= montant;
    comptes[i_dest].solde += montant;

    if (nb_transactions >= MAX_TRANSACTIONS) {
        printf(COLOR_RED "Nombre maximal de transactions atteint.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_operation();
        return;
    }
    Transaction t;
    t.id_transaction = next_transaction_id++;
    t.id_compte = id_source;
    strcpy(t.type, "Virement");
    t.montant = montant;
    t.compte_destination = id_dest;
    transactions[nb_transactions++] = t;

    printf(COLOR_GREEN "Virement reussi!\n" COLOR_RESET);
    printf("Nouveau solde source: %.2f DH\n", comptes[i_source].solde);
    printf("Nouveau solde destination: %.2f DH\n", comptes[i_dest].solde);
    sprintf(log_msg, "Virement: Source=%09d, Dest=%09d, Montant=%.2f, SoldeSource=%.2f, SoldeDest=%.2f",
            id_source, id_dest, montant, comptes[i_source].solde, comptes[i_dest].solde);
    journaliser_evenement(log_msg);
    sauvegarder_donnees();
    pause_ecran();
    Gestion_des_operation();
}

void afficher_historique() {
    int id;
    printf("\n" COLOR_CYAN "--- HISTORIQUE DES TRANSACTIONS ---\n" COLOR_RESET);
    printf("RIB du compte: ");
    scanf("%d", &id);

    /* Vérifier que le compte existe */
    if (chercherCompteParId(id) == -1) {
        ERREUR_RETOUR("Compte introuvable!", Gestion_des_operation);
    }

    /* Vérifier le PIN avant d'afficher l'historique */
    if (!verifier_pin(id)) {
        pause_ecran();
        Gestion_des_operation();
        return;
    }

    if (nb_transactions == 0) {
        printf(COLOR_YELLOW "Aucune transaction enregistree.\n" COLOR_RESET);
        pause_ecran();
        Gestion_des_operation();
        return;
    }

    printf("\n+------+------------+-------------+---------------+----------------+\n");
    printf("| %-4s | %-10s | %-11s | %-13s | %-14s |\n",
           "ID", "RIB", "Type", "Montant (DH)", "Vers RIB");
    printf("+------+------------+-------------+---------------+----------------+\n");

    int count = 0;
    for (int i = 0; i < nb_transactions; i++) {
        if (transactions[i].id_compte == id) {
            char dest[15];
            if (transactions[i].compte_destination != -1) {
                sprintf(dest, "%09d", transactions[i].compte_destination);
            } else {
                strcpy(dest, "-");
            }
            printf("| %-4d | %09d | %-11s | %13.2f | %-14s |\n",
                   transactions[i].id_transaction,
                   transactions[i].id_compte,
                   transactions[i].type,
                   transactions[i].montant,
                   dest);
            count++;
        }
    }
    printf("+------+------------+-------------+---------------+----------------+\n");
    printf("Total: %d transaction(s)\n", count);

    pause_ecran();
    Gestion_des_operation();
}

// ========================================
// FONCTIONS DE SAUVEGARDE/CHARGEMENT
// ========================================

void journaliser_evenement(const char *message) {
    FILE *f = fopen(DB_PATH "historique.txt", "r");
    int file_exists = (f != NULL);

    if (file_exists) {
        fclose(f);
    }

    f = fopen(DB_PATH "historique.txt", "a");
    if (f != NULL) {
        // If file didn't exist, write header first
        if (!file_exists) {
            fprintf(f, "+-----------------------------------------------------------------------------------+\n");
            fprintf(f, "| %-80s |\n", "HISTORIQUE DES EVENEMENTS");
            fprintf(f, "+-----------------------------------------------------------------------------------+\n");
        }

        // Write the event
        fprintf(f, "| %-80s |\n", message);
        fclose(f);
    }
}

void sauvegarder_donnees() {
    FILE *f;

    // Save clients in formatted table
    _mkdir(DB_PATH);

    f = fopen(DB_PATH "clients.txt", "w");
    if (f != NULL) {
        fprintf(f, "# nb_clients=%d next_client_id=%d\n", nb_clients, next_client_id);
        fprintf(f, "+------+----------------------+----------------------+----------------------+---------------+\n");
        fprintf(f, "| %-4s | %-20s | %-20s | %-20s | %-13s |\n",
                "ID", "Nom", "Prenom", "Profession", "Telephone");
        fprintf(f, "+------+----------------------+----------------------+----------------------+---------------+\n");
        for (int i = 0; i < nb_clients; i++) {
            fprintf(f, "| %-4d | %-20s | %-20s | %-20s | %-13s |\n",
                    clients[i].id_client,
                    clients[i].nom,
                    clients[i].prenom,
                    clients[i].profession,
                    clients[i].num_tel);
        }
        fprintf(f, "+------+----------------------+----------------------+----------------------+---------------+\n");
        fclose(f);
    }

    // Save accounts in formatted table
    f = fopen(DB_PATH "comptes.txt", "w");
    if (f != NULL) {
        fprintf(f, "# nb_comptes=%d next_compte_rib=%d\n", nb_comptes, next_compte_rib);
        fprintf(f, "+------------+----------+---------------+------------+------+\n");
        fprintf(f, "| %-10s | %-8s | %-13s | %-10s | %-4s |\n",
                "RIB", "ClientID", "Solde", "Date", "PIN");
        fprintf(f, "+------------+----------+---------------+------------+------+\n");
        for (int i = 0; i < nb_comptes; i++) {
            char date[11];
            sprintf(date, "%02d/%02d/%d",
                    comptes[i].date_ouverture.jour,
                    comptes[i].date_ouverture.mois,
                    comptes[i].date_ouverture.annee);
            fprintf(f, "| %010d | %-8d | %13.2f | %-10s | %-4s |\n",
                    comptes[i].id_compte,
                    comptes[i].id_client,
                    comptes[i].solde,
                    date,
                    comptes[i].pin);
        }
        fprintf(f, "+------------+----------+---------------+------------+------+\n");
        fclose(f);
    }

    // Save transactions in formatted table
    f = fopen(DB_PATH "transactions.txt", "w");
    if (f != NULL) {
        fprintf(f, "# nb_transactions=%d next_transaction_id=%d\n", nb_transactions, next_transaction_id);
        fprintf(f, "+------+------------+-------------+---------------+----------------+\n");
        fprintf(f, "| %-4s | %-10s | %-11s | %-13s | %-14s |\n",
                "ID", "RIB", "Type", "Montant", "DestRIB");
        fprintf(f, "+------+------------+-------------+---------------+----------------+\n");
        for (int i = 0; i < nb_transactions; i++) {
            char dest[15];
            if (transactions[i].compte_destination != -1) {
                sprintf(dest, "%09d", transactions[i].compte_destination);
            } else {
                strcpy(dest, "-");
            }
            fprintf(f, "| %-4d | %010d | %-11s | %13.2f | %-14s |\n",
                    transactions[i].id_transaction,
                    transactions[i].id_compte,
                    transactions[i].type,
                    transactions[i].montant,
                    dest);
        }
        fprintf(f, "+------+------------+-------------+---------------+----------------+\n");
        fclose(f);
    }
}

void charger_donnees() {
    FILE *f;

    // Load clients
    f = fopen(DB_PATH "clients.txt", "r");
    if (f != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), f) != NULL &&
            sscanf(buffer, "# nb_clients=%d next_client_id=%d", &nb_clients, &next_client_id) == 2) {

            if (nb_clients > MAX_CLIENTS) nb_clients = MAX_CLIENTS;

            // Skip the table header lines (3 lines)
            fgets(buffer, sizeof(buffer), f);  // Border line
            fgets(buffer, sizeof(buffer), f);  // Header line
            fgets(buffer, sizeof(buffer), f);  // Border line

            for (int i = 0; i < nb_clients; i++) {
                // Read the formatted line and parse it
                if (fgets(buffer, sizeof(buffer), f) == NULL) {
                    nb_clients = i;
                    break;
                }
                // Parse: | ID | Nom | Prenom | Profession | Telephone |
                if (sscanf(buffer, "| %d | %49s | %49s | %49s | %14s |",
                           &clients[i].id_client,
                           clients[i].nom,
                           clients[i].prenom,
                           clients[i].profession,
                           clients[i].num_tel) != 5) {
                    nb_clients = i;
                    break;
                }
            }
        }
        fclose(f);
    }

    // Load accounts
    f = fopen(DB_PATH "comptes.txt", "r");
    if (f != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), f) != NULL &&
            sscanf(buffer, "# nb_comptes=%d next_compte_rib=%d", &nb_comptes, &next_compte_rib) == 2) {

            if (nb_comptes > MAX_COMPTES) nb_comptes = MAX_COMPTES;

            // Skip header lines
            fgets(buffer, sizeof(buffer), f);
            fgets(buffer, sizeof(buffer), f);
            fgets(buffer, sizeof(buffer), f);

            for (int i = 0; i < nb_comptes; i++) {
                if (fgets(buffer, sizeof(buffer), f) == NULL) {
                    nb_comptes = i;
                    break;
                }
                char date_str[15];
                // Parse: | RIB | ClientID | Solde | Date | PIN |
                if (sscanf(buffer, "| %d | %d | %f | %14s | %4s |",
                           &comptes[i].id_compte,
                           &comptes[i].id_client,
                           &comptes[i].solde,
                           date_str,
                           comptes[i].pin) != 5) {
                    nb_comptes = i;
                    break;
                }
                // Parse date DD/MM/YYYY
                sscanf(date_str, "%d/%d/%d",
                       &comptes[i].date_ouverture.jour,
                       &comptes[i].date_ouverture.mois,
                       &comptes[i].date_ouverture.annee);
            }
        }
        fclose(f);
    }

    // Load transactions
    f = fopen(DB_PATH "transactions.txt", "r");
    if (f != NULL) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), f) != NULL &&
            sscanf(buffer, "# nb_transactions=%d next_transaction_id=%d", &nb_transactions, &next_transaction_id) == 2) {

            if (nb_transactions > MAX_TRANSACTIONS) nb_transactions = MAX_TRANSACTIONS;

            // Skip header lines
            fgets(buffer, sizeof(buffer), f);
            fgets(buffer, sizeof(buffer), f);
            fgets(buffer, sizeof(buffer), f);

            for (int i = 0; i < nb_transactions; i++) {
                if (fgets(buffer, sizeof(buffer), f) == NULL) {
                    nb_transactions = i;
                    break;
                }
                char dest_str[20];
                // Parse: | ID | RIB | Type | Montant | DestRIB |
                if (sscanf(buffer, "| %d | %d | %19s | %f | %19s |",
                           &transactions[i].id_transaction,
                           &transactions[i].id_compte,
                           transactions[i].type,
                           &transactions[i].montant,
                           dest_str) != 5) {
                    nb_transactions = i;
                    break;
                }
                // Parse destination (either number or -)
                if (strcmp(dest_str, "-") == 0) {
                    transactions[i].compte_destination = -1;
                } else {
                    sscanf(dest_str, "%d", &transactions[i].compte_destination);
                }
            }
        }
        fclose(f);
    }
}

// ========================================
// MENU ADMINISTRATEUR
// ========================================

int authentifier_admin() {
    char password[20];
    printf("\n" COLOR_YELLOW "Mot de passe administrateur: " COLOR_RESET);
    scanf("%19s", password);

    if (strcmp(password, "admin123") == 0) {
        printf(COLOR_GREEN "Acces autorise!\n" COLOR_RESET);
        return 1;
    }
    printf(COLOR_RED "Mot de passe incorrect!\n" COLOR_RESET);
    pause_ecran();
    return 0;
}

void afficher_tous_clients_detaille() {
    printf("\n" COLOR_MAGENTA "=======================================\n");
    printf("||    TOUS LES CLIENTS (DETAILLE)    ||\n");
    printf("=======================================\n" COLOR_RESET);

    if (nb_clients == 0) {
        printf("\n" COLOR_YELLOW "Aucun client enregistre dans le systeme.\n" COLOR_RESET);
        pause_ecran();
        menu_admin();
        return;
    }

    printf("\n");
    printf("+------+----------------------+----------------------+----------------------+---------------+----------+\n");
    printf("| %-4s | %-20s | %-20s | %-20s | %-13s | %-8s |\n",
           "ID", "Nom", "Prenom", "Profession", "Telephone", "Comptes");
    printf("+------+----------------------+----------------------+----------------------+---------------+----------+\n");

    for (int i = 0; i < nb_clients; i++) {
        printf("| %-4d | %-20s | %-20s | %-20s | %-13s | %-8d |\n",
               clients[i].id_client,
               clients[i].nom,
               clients[i].prenom,
               clients[i].profession,
               clients[i].num_tel,
               compter_comptes_client(clients[i].id_client));
    }
    printf("+------+----------------------+----------------------+----------------------+---------------+----------+\n");
    printf("\nTotal: %d clients\n", nb_clients);

    pause_ecran();
    menu_admin();
}

void afficher_tous_comptes_detaille() {
    printf("\n" COLOR_MAGENTA "=======================================\n");
    printf("||    TOUS LES COMPTES (DETAILLE)     ||\n");
    printf("=======================================\n" COLOR_RESET);

    if (nb_comptes == 0) {
        printf("\n" COLOR_YELLOW "Aucun compte enregistre dans le systeme.\n" COLOR_RESET);
        pause_ecran();
        menu_admin();
        return;
    }

    printf("\n");
    printf("+------------+----------+----------------------+---------------+------------+------+\n");
    printf("| %-10s  | %-8s | %-20s | %-13s | %-10s | %-4s |\n",
           "RIB", "Client", "Nom Client", "Solde (DH)", "Date", "PIN");
    printf("+------------+----------+----------------------+---------------+------------+------+\n");

    for (int i = 0; i < nb_comptes; i++) {
        int idx = chercherClientParId(comptes[i].id_client);
        char nom_complet[50];
        char date[11];

        if (idx != -1) {
            sprintf(nom_complet, "%s %s", clients[idx].nom, clients[idx].prenom);
        } else {
            strcpy(nom_complet, "Inconnu");
        }

        sprintf(date, "%02d/%02d/%d",
                comptes[i].date_ouverture.jour,
                comptes[i].date_ouverture.mois,
                comptes[i].date_ouverture.annee);

        printf("| %010d | %-8d | %-20s | %13.2f | %-10s | %-4s |\n",
               comptes[i].id_compte,
               comptes[i].id_client,
               nom_complet,
               comptes[i].solde,
               date,
               comptes[i].pin);
    }
    printf("+------------+----------+----------------------+---------------+------------+------+\n");
    printf("\nTotal: %d comptes\n", nb_comptes);

    pause_ecran();
    menu_admin();
}

void afficher_toutes_transactions_detaille() {
    printf("\n" COLOR_MAGENTA "=======================================\n");
    printf("|| TOUTES LES TRANSACTIONS (DETAILLE) ||\n");
    printf("=======================================\n" COLOR_RESET);

    if (nb_transactions == 0) {
        printf("\n" COLOR_YELLOW "Aucune transaction enregistree.\n" COLOR_RESET);
        pause_ecran();
        menu_admin();
        return;
    }

    printf("\n");
    printf("+------+------------+-------------+---------------+----------------+\n");
    printf("| %-4s | %-10s | %-11s | %-13s | %-14s |\n",
           "ID", "RIB", "Type", "Montant (DH)", "Vers RIB");
    printf("+------+------------+-------------+---------------+----------------+\n");

    for (int i = 0; i < nb_transactions; i++) {
        char dest[15];
        if (transactions[i].compte_destination != -1) {
            sprintf(dest, "%010d", transactions[i].compte_destination);
        } else {
            strcpy(dest, "-");
        }

        printf("| %-4d | %010d | %-11s | %13.2f | %-14s |\n",
               transactions[i].id_transaction,
               transactions[i].id_compte,
               transactions[i].type,
               transactions[i].montant,
               dest);
    }
    printf("+------+------------+-------------+---------------+----------------+\n");
    printf("\nTotal: %d transactions\n", nb_transactions);

    pause_ecran();
    menu_admin();
}

void afficher_statistiques() {
    printf("\n" COLOR_MAGENTA "=======================================\n");
    printf("||         STATISTIQUES SYSTEME       ||\n");
    printf("=======================================\n" COLOR_RESET);

    // Calculate total money in system
    float solde_total = 0;
    for (int i = 0; i < nb_comptes; i++) {
        solde_total += comptes[i].solde;
    }

    // Calculate average balance
    float solde_moyen = (nb_comptes > 0) ? solde_total / nb_comptes : 0;

    // Count transaction types
    int nb_depots = 0, nb_retraits = 0, nb_virements = 0;
    float total_depots = 0, total_retraits = 0, total_virements = 0;

    for (int i = 0; i < nb_transactions; i++) {
        if (strcmp(transactions[i].type, "Depot") == 0) {
            nb_depots++;
            total_depots += transactions[i].montant;
        } else if (strcmp(transactions[i].type, "Retrait") == 0) {
            nb_retraits++;
            total_retraits += transactions[i].montant;
        } else if (strcmp(transactions[i].type, "Virement") == 0) {
            nb_virements++;
            total_virements += transactions[i].montant;
        }
    }

    printf("\n" COLOR_CYAN "CLIENTS ET COMPTES:\n" COLOR_RESET);
    printf("  Nombre total de clients: %d\n", nb_clients);
    printf("  Nombre total de comptes: %d\n", nb_comptes);
    printf("  Solde total dans le systeme: " COLOR_GREEN "%.2f DH\n" COLOR_RESET, solde_total);
    printf("  Solde moyen par compte: %.2f DH\n", solde_moyen);

    printf("\n" COLOR_CYAN "TRANSACTIONS:\n" COLOR_RESET);
    printf("  Total transactions: %d\n", nb_transactions);
    printf("  Depots: %d (Total: %.2f DH)\n", nb_depots, total_depots);
    printf("  Retraits: %d (Total: %.2f DH)\n", nb_retraits, total_retraits);
    printf("  Virements: %d (Total: %.2f DH)\n", nb_virements, total_virements);

    // Find richest account
    if (nb_comptes > 0) {
        int idx_max = 0;
        for (int i = 1; i < nb_comptes; i++) {
            if (comptes[i].solde > comptes[idx_max].solde) {
                idx_max = i;
            }
        }
        printf("\n" COLOR_CYAN "COMPTE LE PLUS RICHE:\n" COLOR_RESET);
        printf("  RIB: %09d\n", comptes[idx_max].id_compte);
        printf("  Solde: " COLOR_GREEN "%.2f DH\n" COLOR_RESET, comptes[idx_max].solde);
    }

    pause_ecran();
    menu_admin();
}

void menu_admin() {
    int choix;

    char *titre =
        "\n" COLOR_MAGENTA "=======================================\n"
        "||      MENU ADMINISTRATEUR          ||\n"
        "=======================================\n" COLOR_RESET;

    char *options[] = {
        "Voir tous les clients",
        "Voir tous les comptes",
        "Voir toutes les transactions",
        "Voir statistiques du systeme",  // ADD THIS
        "Retour au menu principal"
    };
    choix = menu_avec_fleches(titre, options, 5);  // Change 4 to 5
    switch(choix) {
        case 1: afficher_tous_clients_detaille(); break;
        case 2: afficher_tous_comptes_detaille(); break;
        case 3: afficher_toutes_transactions_detaille(); break;
        case 4: afficher_statistiques(); break;  // ADD THIS
        case 5: menu_principale(); break;  // Change 4 to 5
    }
}

void Gestion_des_clients() {
    int b;

    char *titre =
        "\n" COLOR_BLUE "**************************************\n"
        "*       GESTION DES CLIENTS          *\n"
        "**************************************\n" COLOR_RESET;

    char *options[] = {
        "Ajouter un client",
        "Modifier un client",
        "Supprimer un client",
        "Rechercher un client",
        "Retour au menu principal"
    };

    b = menu_avec_fleches(titre, options, 5);

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

    char *titre =
        "\n" COLOR_BLUE "**************************************\n"
        "*       GESTION DES COMPTES          *\n"
        "**************************************\n" COLOR_RESET;

    char *options[] = {
        "Nouveau compte",
        "Consultation",
        "Fermeture du compte",
        "Retour au menu principal"
    };

    b = menu_avec_fleches(titre, options, 4);

    switch(b) {
        case 1: Nouveau_compte(); break;
        case 2: consultation(); break;
        case 3: fermeture_du_compte(); break;
        case 4: menu_principale(); break;
    }
}

void Gestion_des_operation() {
    int b;

    char *titre =
        "\n" COLOR_BLUE "**************************************\n"
        "*      GESTION DES OPERATIONS        *\n"
        "**************************************\n" COLOR_RESET;

    char *options[] = {
        "Depot",
        "Retrait",
        "Virement",
        "Historique des transactions",
        "Retour au menu principal"
    };

    b = menu_avec_fleches(titre, options, 5);

    switch(b) {
        case 1: Depot(); break;
        case 2: Retrait(); break;
        case 3: Virement(); break;
        case 4: afficher_historique(); break;
        case 5: menu_principale(); break;
    }
}

void menu_principale() {
    int a;
    char secret_input[20];

    char *titre =
        "\n" COLOR_CYAN "======================================\n"
        "||    SYSTEME DE GESTION BANCAIRE   ||\n"
        "======================================\n" COLOR_RESET
        COLOR_BLUE "||          MENU PRINCIPAL          ||\n" COLOR_RESET
        COLOR_CYAN "======================================\n" COLOR_RESET;

    char *options[] = {
        "Gestion des clients",
        "Gestion des comptes",
        "Gestion des operations",
        "Quitter"
    };

    while(1) {
        a = menu_avec_fleches(titre, options, 4);

        // Check if user wants to access admin menu
        if(a == -999) {
            clear_screen();
            if (authentifier_admin()) {
                menu_admin();
            }
            continue;
        }

        switch(a) {
            case 1:
                Gestion_des_clients();
                break;
            case 2:
                Gestion_des_comptes();
                break;
            case 3:
                Gestion_des_operation();
                break;
            case 4:
                sauvegarder_donnees();
                clear_screen();
                printf("\n" COLOR_GREEN "Merci d'avoir utilise notre systeme!\n" COLOR_RESET);
                printf(COLOR_GREEN "Au revoir!\n" COLOR_RESET);
                exit(0);
        }
    }
}

void run_application() {
    afficher_chargement("Chargement des donnees");
    charger_donnees();
    menu_principale();
}

int main() {
    run_application();
    return 0;
}
