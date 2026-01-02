#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

/*
 * Mini-projet C : Gestion des Clients et des Comptes Bancaires
 * - Gestion des clients (ajout, modification, suppression, recherche)
 * - Gestion des comptes (creation, consultation, fermeture)
 * - Operations financieres (versement, retrait, virement)
 * - Historique des transactions
 * - Sauvegarde/chargement dans des fichiers binaires + export texte
 * - Donnees stockees dans "database"
 *   et exports lisibles dans "database/backdoor acces"
 */

/* Dossiers de stockage */
#define DB_DIR           "database"
#define BACKDOOR_DIR     "database/backdoor acces"

/* ========== STRUCTURES ========== */

/* Informations sur un client de la banque */
typedef struct {
    int id_client;
    char nom[50];
    char prenom[50];
    char profession[50];
    char num_tel[15];
} Client;

/* Informations sur un compte bancaire */
typedef struct {
    int id_compte;
    int id_client;          /* proprietaire du compte */
    float solde;
    char date_ouverture[20];
    char pin[5];            /* PIN a 4 chiffres */
} Compte;

/* Informations sur une transaction effectuee sur un compte */
typedef struct {
    int id_transaction;
    int id_compte;
    char type[20];          /* "Retrait", "Virement", "Versement" */
    float montant;
    char date[30];
    char details[150];
    int compte_destination; /* id compte lie (pour virement) ou -1 */
} Transaction;

/* ========== VARIABLES GLOBALES ========== */

Client *clients = NULL;
Compte *comptes = NULL;
Transaction *transactions = NULL;
int nb_clients = 0;
int nb_comptes = 0;
int nb_transactions = 0;
int next_transaction_id = 1;

/* ========== PROTOTYPES ========== */

// Menus
void menu_principale();
void Gestion_des_clients();
void Gestion_des_comptes();
void Gestion_des_operation();
void menu_admin();
int authentifier_admin();

// Utilitaires generaux
void clear_buffer();
void pause_screen();
void obtenir_date_heure(char *buffer);
int valider_date(char date[]);
int valider_telephone(char tel[]);
int valider_nom(char nom[]);
int telephone_existe(char tel[], int exclude_id);
int verifier_pin(int id_compte);

// Saisie / confirmation
int saisir_int(const char *message);
float saisir_float(const char *message);
void saisir_chaine(const char *message, char *buffer, size_t taille);
void saisir_nom_valide(const char *message, char *dest);
void saisir_telephone_valide(char *dest, int exclude_id);
void saisir_date_valide(const char *message, char *dest, size_t taille);
int confirmer(const char *message);

// Clients / comptes utilitaires
int chercherClientParId(int id);
int chercherClientParNom(char nom[]);
int compter_comptes_client(int id_client);
int chercherCompteParId(int id);
void supprimer_client_par_index(int index);
void supprimer_compte_par_index(int index);

// Operations clients
void Ajoute_un_client();
void Modifications();
void Suppression();
void Recherche();

// Operations comptes
void Nouveau_compte();
void consultation();
void fermeture_du_compte();

// Operations financieres
void Versement();
void Retrait();
void Virement();

// Transactions
void ajouter_transaction(int id_compte, char type[], float montant, char details[], int compte_dest);
void afficher_historique();
void afficher_historique_compte(int id_compte);
void afficher_une_transaction(const Transaction *t);

// Persistance
void sauvegarder_donnees();
void charger_donnees();
void exporter_tout_txt();
void ensure_directories();

// Admin functions
void afficher_statistiques();
void afficher_tous_clients_detaille();
void afficher_tous_comptes_detaille();
void afficher_toutes_transactions_detaille();
void afficher_client_complet(int id_client);
void afficher_compte_complet(int id_compte);
void informations_systeme();

// Lancement global de l'application
void run_application();

/* ========== UTILITAIRES GENERAUX ========== */

void clear_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void pause_screen() {
    printf("\nAppuyez sur Entree pour continuer...");
    clear_buffer();
    getchar();
}

void obtenir_date_heure(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 30, "%d/%m/%Y %H:%M:%S", t);
}

int valider_date(char date[]) {
    if (strlen(date) != 10) return 0;
    if (date[2] != '/' || date[5] != '/') return 0;

    for (int i = 0; i < 10; i++) {
        if (i == 2 || i == 5) continue;
        if (!isdigit((unsigned char)date[i])) return 0;
    }

    int jour = (date[0] - '0') * 10 + (date[1] - '0');
    int mois = (date[3] - '0') * 10 + (date[4] - '0');
    int annee = (date[6] - '0') * 1000 + (date[7] - '0') * 100 +
                (date[8] - '0') * 10 + (date[9] - '0');

    if (jour < 1 || jour > 31) return 0;
    if (mois < 1 || mois > 12) return 0;
    if (annee < 1900 || annee > 2100) return 0;

    return 1;
}

int valider_telephone(char tel[]) {
    size_t len = strlen(tel);
    if (len < 10 || len > 14) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)tel[i])) return 0;
    }
    return 1;
}

int valider_nom(char nom[]) {
    size_t len = strlen(nom);
    if (len == 0) return 0;
    for (size_t i = 0; i < len; i++) {
        if (isdigit((unsigned char)nom[i])) return 0;
    }
    return 1;
}

int telephone_existe(char tel[], int exclude_id) {
    for (int i = 0; i < nb_clients; i++) {
        if (clients[i].id_client != exclude_id &&
            strcmp(clients[i].num_tel, tel) == 0) {
            return 1;
        }
    }
    return 0;
}

int verifier_pin(int id_compte) {
    int idx = chercherCompteParId(id_compte);
    if (idx == -1) return 0;

    char pin[5];
    int tentatives = 0;

    while (tentatives < 3) {
        printf("\nEntrez le code PIN (4 chiffres): ");
        scanf("%4s", pin);
        clear_buffer();

        if (strcmp(pin, comptes[idx].pin) == 0) {
            printf(" [OK] PIN correct\n");
            return 1;
        }

        tentatives++;
        printf(" [ERREUR] PIN incorrect. Tentatives restantes: %d\n", 3 - tentatives);
    }

    printf("\n [ATTENTION] Compte bloque temporairement\n");
    return 0;
}

/* ========== SAISIE / CONFIRMATION ========== */

int saisir_int(const char *message) {
    int x;
    printf("%s", message);
    scanf("%d", &x);
    clear_buffer();
    return x;
}

float saisir_float(const char *message) {
    float x;
    printf("%s", message);
    scanf("%f", &x);
    clear_buffer();
    return x;
}

void saisir_chaine(const char *message, char *buffer, size_t taille) {
    printf("%s", message);
    scanf("%s", buffer);
    buffer[taille - 1] = '\0';
    clear_buffer();
}

void saisir_nom_valide(const char *message, char *dest) {
    do {
        saisir_chaine(message, dest, 50);
        if (!valider_nom(dest)) {
            printf(" [ERREUR] Nom invalide (pas de chiffres)\n");
        }
    } while (!valider_nom(dest));
}

void saisir_telephone_valide(char *dest, int exclude_id) {
    do {
        saisir_chaine("Telephone (10-14 chiffres): ", dest, 15);
        if (!valider_telephone(dest)) {
            printf(" [ERREUR] Telephone invalide\n");
        } else if (telephone_existe(dest, exclude_id)) {
            printf(" [ERREUR] Ce numero existe deja\n");
        } else {
            break;
        }
    } while (1);
}

void saisir_date_valide(const char *message, char *dest, size_t taille) {
    char tmp[32];
    do {
        saisir_chaine(message, tmp, sizeof(tmp));
        if (valider_date(tmp)) {
            strncpy(dest, tmp, taille);
            dest[taille - 1] = '\0';
            break;
        }
        printf(" [ERREUR] Format invalide. Utilisez JJ/MM/AAAA\n");
    } while (1);
}

int confirmer(const char *message) {
    char rep;
    printf("%s (O/N): ", message);
    scanf(" %c", &rep);
    clear_buffer();
    return (rep == 'O' || rep == 'o');
}

/* ========== CLIENTS / COMPTES UTILITAIRES ========== */

int chercherClientParId(int id) {
    for (int i = 0; i < nb_clients; i++)
        if (clients[i].id_client == id)
            return i;
    return -1;
}

int chercherClientParNom(char nom[]) {
    for (int i = 0; i < nb_clients; i++)
        if (strcmp(clients[i].nom, nom) == 0)
            return i;
    return -1;
}

int compter_comptes_client(int id_client) {
    int count = 0;
    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_client == id_client)
            count++;
    }
    return count;
}

void supprimer_client_par_index(int index) {
    for (int j = index; j < nb_clients - 1; j++)
        clients[j] = clients[j + 1];

    nb_clients--;
    if (nb_clients > 0)
        clients = realloc(clients, nb_clients * sizeof(Client));
    else {
        free(clients);
        clients = NULL;
    }
}

int chercherCompteParId(int id) {
    for (int i = 0; i < nb_comptes; i++)
        if (comptes[i].id_compte == id)
            return i;
    return -1;
}

void supprimer_compte_par_index(int index) {
    for (int j = index; j < nb_comptes - 1; j++)
        comptes[j] = comptes[j + 1];

    nb_comptes--;
    if (nb_comptes > 0)
        comptes = realloc(comptes, nb_comptes * sizeof(Compte));
    else {
        free(comptes);
        comptes = NULL;
    }
}

/* ========== OPERATIONS CLIENTS ========== */

void Ajoute_un_client() {
    Client c;
    printf("\n--- AJOUTER UN CLIENT ---\n");

    c.id_client = saisir_int("Id client: ");
    if (chercherClientParId(c.id_client) != -1) {
        printf("\n [ERREUR] Client existe deja\n");
        pause_screen();
        return;
    }

    saisir_nom_valide("Nom: ", c.nom);
    saisir_nom_valide("Prenom: ", c.prenom);
    saisir_chaine("Profession: ", c.profession, sizeof(c.profession));
    saisir_telephone_valide(c.num_tel, -1);

    clients = realloc(clients, (nb_clients + 1) * sizeof(Client));
    clients[nb_clients++] = c;

    printf("\n [OK] Client ajoute avec succes\n");
    pause_screen();
}

void Modifications() {
    int id, choix;
    printf("\n--- MODIFIER UN CLIENT ---\n");

    id = saisir_int("Entrer Id client: ");
    int i = chercherClientParId(id);
    if (i == -1) {
        printf("\n [ERREUR] Client introuvable\n");
        pause_screen();
        return;
    }

    printf("\nClient actuel: %s %s\n", clients[i].nom, clients[i].prenom);
    printf("1. Nom\n");
    printf("2. Prenom\n");
    printf("3. Profession\n");
    printf("4. Telephone\n");
    printf("Votre choix: ");
    scanf("%d", &choix);
    clear_buffer();

    char temp[50];
    switch (choix) {
        case 1:
            saisir_nom_valide("Nouveau nom: ", clients[i].nom);
            break;
        case 2:
            saisir_nom_valide("Nouveau prenom: ", clients[i].prenom);
            break;
        case 3:
            saisir_chaine("Nouvelle profession: ", clients[i].profession, sizeof(clients[i].profession));
            break;
        case 4:
            saisir_telephone_valide(temp, id);
            strcpy(clients[i].num_tel, temp);
            break;
        default:
            printf("\n [ERREUR] Choix invalide\n");
            pause_screen();
            return;
    }

    printf("\n [OK] Modification reussie\n");
    pause_screen();
}

void Suppression() {
    int id;
    printf("\n--- SUPPRIMER UN CLIENT ---\n");

    id = saisir_int("Entrer Id client: ");
    int i = chercherClientParId(id);
    if (i == -1) {
        printf("\n [ERREUR] Client introuvable\n");
        pause_screen();
        return;
    }

    int nb_comptes_cl = compter_comptes_client(id);
    if (nb_comptes_cl > 0) {
        printf("\n [ATTENTION] Ce client possede %d compte(s)\n", nb_comptes_cl);
        if (!confirmer("Voulez-vous vraiment supprimer ce client")) {
            printf("\n Suppression annulee\n");
            pause_screen();
            return;
        }
    }

    printf("Client: %s %s\n", clients[i].nom, clients[i].prenom);
    if (!confirmer("Confirmer suppression")) {
        printf("\n Suppression annulee\n");
        pause_screen();
        return;
    }

    supprimer_client_par_index(i);
    printf("\n [OK] Client supprime\n");
    pause_screen();
}

void Recherche() {
    int choix;
    printf("\n--- RECHERCHER UN CLIENT ---\n");
    printf("1. Recherche par ID\n");
    printf("2. Recherche par Nom\n");
    printf("Votre choix: ");
    scanf("%d", &choix);
    clear_buffer();

    if (choix == 1) {
        int id = saisir_int("Id client: ");
        int i = chercherClientParId(id);
        if (i != -1) {
            printf("\n [OK] Client trouve:\n");
            printf("ID: %d | %s %s | %s | Tel: %s\n",
                   clients[i].id_client,
                   clients[i].nom,
                   clients[i].prenom,
                   clients[i].profession,
                   clients[i].num_tel);
            printf("Nombre de comptes: %d\n", compter_comptes_client(id));
        } else {
            printf("\n [ERREUR] Client introuvable\n");
        }
    } else if (choix == 2) {
        char nom[50];
        saisir_chaine("Nom: ", nom, sizeof(nom));
        int i = chercherClientParNom(nom);
        if (i != -1) {
            printf("\n [OK] Client trouve:\n");
            printf("ID: %d | %s %s | %s | Tel: %s\n",
                   clients[i].id_client,
                   clients[i].nom,
                   clients[i].prenom,
                   clients[i].profession,
                   clients[i].num_tel);
            printf("Nombre de comptes: %d\n", compter_comptes_client(clients[i].id_client));
        } else {
            printf("\n [ERREUR] Client introuvable\n");
        }
    } else {
        printf("\n [ERREUR] Choix invalide\n");
    }
    pause_screen();
}

/* ========== OPERATIONS COMPTES ========== */

void Nouveau_compte() {
    Compte c;
    printf("\n--- CREER UN COMPTE ---\n");

    c.id_compte = saisir_int("Id compte: ");
    if (chercherCompteParId(c.id_compte) != -1) {
        printf("\n [ERREUR] Compte existe deja\n");
        pause_screen();
        return;
    }

    c.id_client = saisir_int("Id client: ");
    int idx = chercherClientParId(c.id_client);
    if (idx == -1) {
        printf("\n [ERREUR] Client inexistant\n");
        pause_screen();
        return;
    }

    printf("\nClient: %s %s\n", clients[idx].nom, clients[idx].prenom);

    c.solde = saisir_float("Solde initial (min 1000 DH): ");
    if (c.solde < 1000) {
        printf("\n [ERREUR] Solde minimum 1000 DH\n");
        pause_screen();
        return;
    }

    saisir_date_valide("Date d'ouverture (JJ/MM/AAAA): ", c.date_ouverture, sizeof(c.date_ouverture));

    /* Configuration du PIN */
    do {
        printf("Definir un code PIN (4 chiffres): ");
        scanf("%4s", c.pin);
        clear_buffer();

        if (strlen(c.pin) != 4) {
            printf(" [ERREUR] Le PIN doit contenir 4 chiffres\n");
            continue;
        }

        int valid = 1;
        for (int i = 0; i < 4; i++) {
            if (!isdigit((unsigned char)c.pin[i])) {
                valid = 0;
                break;
            }
        }

        if (valid) {
            char confirm[5];
            printf("Confirmer le PIN: ");
            scanf("%4s", confirm);
            clear_buffer();

            if (strcmp(c.pin, confirm) == 0) {
                break;
            } else {
                printf(" [ERREUR] Les PINs ne correspondent pas\n");
            }
        } else {
            printf(" [ERREUR] Le PIN doit contenir uniquement des chiffres\n");
        }
    } while (1);

    comptes = realloc(comptes, (nb_comptes + 1) * sizeof(Compte));
    comptes[nb_comptes++] = c;

    printf("\n [OK] Compte cree avec succes\n");
    printf(" [SECURITE] PIN configure: ****\n");

    /* Enregistrer le versement initial comme transaction */
    char details[150];
    sprintf(details, "Versement initial - Ouverture du compte");
    ajouter_transaction(c.id_compte, "Versement", c.solde, details, -1);

    pause_screen();
}

void consultation() {
    int id;
    printf("\n--- CONSULTATION ---\n");
    
    id = saisir_int("Id compte: ");
    int i = chercherCompteParId(id);
    
    if (i == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }
    
    /* Verification du PIN obligatoire */
    printf("\n [SECURITE] Authentification requise\n");
    if (!verifier_pin(id)) {
        printf("\n [ERREUR] Acces refuse - PIN incorrect\n");
        pause_screen();
        return;
    }
    
    /* Afficher les informations du compte */
    int idx = chercherClientParId(comptes[i].id_client);
    printf("\n [OK] Compte trouve:\n");
    printf("======================================\n");
    printf("Compte: %d\n", comptes[i].id_compte);
    printf("Client: %s %s (ID: %d)\n",
           idx != -1 ? clients[idx].nom : "Inconnu",
           idx != -1 ? clients[idx].prenom : "",
           comptes[i].id_client);
    printf("Solde: %.2f DH\n", comptes[i].solde);
    printf("Date ouverture: %s\n", comptes[i].date_ouverture);
    printf("Securite: PIN active (****)\n");
    printf("======================================\n");
    
    pause_screen();
}

void fermeture_du_compte() {
    printf("\n--- FERMER UN COMPTE ---\n");
    int id = saisir_int("Id compte: ");

    int i = chercherCompteParId(id);
    if (i == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }

    printf("\n [SECURITE] Verification de securite requise\n");
    if (!verifier_pin(id)) {
        printf("\n [ERREUR] Fermeture annulee\n");
        pause_screen();
        return;
    }

    printf("\nCompte %d | Solde: %.2f DH\n", comptes[i].id_compte, comptes[i].solde);
    if (!confirmer("Confirmer suppression")) {
        printf("\n Suppression annulee\n");
        pause_screen();
        return;
    }

    supprimer_compte_par_index(i);
    printf("\n [OK] Compte supprime\n");
    pause_screen();
}

/* ========== OPERATIONS FINANCIERES ========== */

void Versement() {
    int id;
    float montant;

    printf("\n--- VERSEMENT ---\n");
    id = saisir_int("Id compte: ");

    int i = chercherCompteParId(id);
    if (i == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }

    printf("Solde actuel: %.2f DH\n", comptes[i].solde);
    montant = saisir_float("Montant a verser: ");

    if (montant <= 0) {
        printf("\n [ERREUR] Montant invalide\n");
        pause_screen();
        return;
    }

    comptes[i].solde += montant;

    char details[150];
    sprintf(details, "Versement de %.2f DH", montant);
    ajouter_transaction(id, "Versement", montant, details, -1);

    printf("\n [OK] Versement effectue | Nouveau solde: %.2f DH\n", comptes[i].solde);
    pause_screen();
}

void Retrait() {
    int id;
    float m;

    printf("\n--- RETRAIT ---\n");
    id = saisir_int("Id compte: ");

    int i = chercherCompteParId(id);
    if (i == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }

    printf("\n [SECURITE] Authentification requise\n");
    if (!verifier_pin(id)) {
        printf("\n [ERREUR] Operation annulee\n");
        pause_screen();
        return;
    }

    printf("\nSolde actuel: %.2f DH\n", comptes[i].solde);
    m = saisir_float("Montant a retirer: ");

    if (m <= 0) {
        printf("\n [ERREUR] Montant invalide\n");
        pause_screen();
        return;
    }

    if (m > 700) {
        printf("\n [ERREUR] Limite de retrait: 700 DH maximum\n");
        pause_screen();
        return;
    }

    if (comptes[i].solde < m) {
        printf("\n [ERREUR] Solde insuffisant\n");
        pause_screen();
        return;
    }

    comptes[i].solde -= m;

    char details[150];
    sprintf(details, "Retrait de %.2f DH", m);
    ajouter_transaction(id, "Retrait", m, details, -1);

    printf("\n [OK] Retrait effectue | Nouveau solde: %.2f DH\n", comptes[i].solde);
    pause_screen();
}

void Virement() {
    int c1, c2;
    float m;

    printf("\n--- VIREMENT ---\n");

    c1 = saisir_int("Compte source: ");
    int i1 = chercherCompteParId(c1);
    if (i1 == -1) {
        printf("\n [ERREUR] Compte source introuvable\n");
        pause_screen();
        return;
    }

    printf("\n [SECURITE] Authentification requise\n");
    if (!verifier_pin(c1)) {
        printf("\n [ERREUR] Operation annulee\n");
        pause_screen();
        return;
    }

    c2 = saisir_int("Compte destination: ");
    int i2 = chercherCompteParId(c2);
    if (i2 == -1) {
        printf("\n [ERREUR] Compte destination introuvable\n");
        pause_screen();
        return;
    }

    if (c1 == c2) {
        printf("\n [ERREUR] Les comptes doivent etre differents\n");
        pause_screen();
        return;
    }

    printf("Solde compte source: %.2f DH\n", comptes[i1].solde);
    m = saisir_float("Montant a virer: ");

    if (m <= 0) {
        printf("\n [ERREUR] Montant invalide\n");
        pause_screen();
        return;
    }

    if (comptes[i1].solde < m) {
        printf("\n [ERREUR] Solde insuffisant\n");
        pause_screen();
        return;
    }

    comptes[i1].solde -= m;
    comptes[i2].solde += m;

    char details1[150], details2[150];
    sprintf(details1, "Virement vers compte %d de %.2f DH", c2, m);
    sprintf(details2, "Virement recu du compte %d de %.2f DH", c1, m);
    ajouter_transaction(c1, "Virement", m, details1, c2);
    ajouter_transaction(c2, "Virement", m, details2, c1);

    printf("\n [OK] Virement reussi\n");
    printf("Nouveau solde source: %.2f DH\n", comptes[i1].solde);
    printf("Nouveau solde destination: %.2f DH\n", comptes[i2].solde);
    pause_screen();
}

/* ========== TRANSACTIONS ========== */

void ajouter_transaction(int id_compte, char type[], float montant, char details[], int compte_dest) {
    transactions = realloc(transactions, (nb_transactions + 1) * sizeof(Transaction));

    transactions[nb_transactions].id_transaction = next_transaction_id++;
    transactions[nb_transactions].id_compte = id_compte;
    strcpy(transactions[nb_transactions].type, type);
    transactions[nb_transactions].montant = montant;
    strcpy(transactions[nb_transactions].details, details);
    transactions[nb_transactions].compte_destination = compte_dest;
    obtenir_date_heure(transactions[nb_transactions].date);

    nb_transactions++;
}

void afficher_une_transaction(const Transaction *t) {
    printf("\n[Transaction #%d]\n", t->id_transaction);
    printf("Compte: %d\n", t->id_compte);
    printf("Type: %s\n", t->type);
    printf("Montant: %.2f DH\n", t->montant);
    printf("Date: %s\n", t->date);
    printf("Details: %s\n", t->details);
    if (t->compte_destination != -1) {
        printf("Compte lie: %d\n", t->compte_destination);
    }
    printf("--------------------------------------");
}

void afficher_historique_compte(int id_compte) {
    int found = 0;

    printf("\n======================================\n");
    printf("  HISTORIQUE DU COMPTE %d\n", id_compte);
    printf("======================================\n");

    for (int i = 0; i < nb_transactions; i++) {
        if (transactions[i].id_compte == id_compte) {
            afficher_une_transaction(&transactions[i]);
            found = 1;
        }
    }

    if (!found) {
        printf("\n Aucune transaction pour ce compte\n");
    }
    printf("\n======================================\n");
}

void afficher_historique() {
    int id;
    printf("\n--- HISTORIQUE DES TRANSACTIONS ---\n");
    
    id = saisir_int("Id compte: ");
    
    if (chercherCompteParId(id) == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }
    
    /* Verification du PIN obligatoire */
    printf("\n [SECURITE] Authentification requise\n");
    if (!verifier_pin(id)) {
        printf("\n [ERREUR] Acces refuse - PIN incorrect\n");
        pause_screen();
        return;
    }
    
    /* Afficher l'historique du compte */
    afficher_historique_compte(id);
}

/* ========== PERSISTANCE ========== */

void ensure_directories() {
    MKDIR(DB_DIR);
    MKDIR(BACKDOOR_DIR);
}

/* Export des listes en fichiers texte lisibles (.txt) dans BACKDOOR_DIR */
void exporter_tout_txt() {
    FILE *f;
    int i;

    ensure_directories();

    f = fopen(BACKDOOR_DIR "/clients.txt", "w");
    if (f != NULL) {
        fprintf(f, "ID_CLIENT;NOM;PRENOM;PROFESSION;TELEPHONE\n");
        for (i = 0; i < nb_clients; i++) {
            fprintf(f, "%d;%s;%s;%s;%s\n",
                    clients[i].id_client,
                    clients[i].nom,
                    clients[i].prenom,
                    clients[i].profession,
                    clients[i].num_tel);
        }
        fclose(f);
    }

    f = fopen(BACKDOOR_DIR "/comptes.txt", "w");
    if (f != NULL) {
        fprintf(f, "ID_COMPTE;ID_CLIENT;SOLDE;DATE_OUVERTURE\n");
        for (i = 0; i < nb_comptes; i++) {
            fprintf(f, "%d;%d;%.2f;%s\n",
                    comptes[i].id_compte,
                    comptes[i].id_client,
                    comptes[i].solde,
                    comptes[i].date_ouverture);
        }
        fclose(f);
    }

    f = fopen(BACKDOOR_DIR "/transactions.txt", "w");
    if (f != NULL) {
        fprintf(f, "ID_TRANSACTION;ID_COMPTE;TYPE;MONTANT;DATE;DETAILS;COMPTE_LIE\n");
        for (i = 0; i < nb_transactions; i++) {
            fprintf(f, "%d;%d;%s;%.2f;%s;%s;%d\n",
                    transactions[i].id_transaction,
                    transactions[i].id_compte,
                    transactions[i].type,
                    transactions[i].montant,
                    transactions[i].date,
                    transactions[i].details,
                    transactions[i].compte_destination);
        }
        fclose(f);
    }
}

/* Sauvegarde en binaire + export texte dans le dossier database (SILENCIEUX) */
void sauvegarder_donnees() {
    FILE *f;

    ensure_directories();

    /* Clients */
    f = fopen(DB_DIR "/clients.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_clients, sizeof(int), 1, f);
        if (nb_clients > 0) {
            fwrite(clients, sizeof(Client), nb_clients, f);
        }
        fclose(f);
    }

    /* Comptes */
    f = fopen(DB_DIR "/comptes.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_comptes, sizeof(int), 1, f);
        if (nb_comptes > 0) {
            fwrite(comptes, sizeof(Compte), nb_comptes, f);
        }
        fclose(f);
    }

    /* Transactions */
    f = fopen(DB_DIR "/transactions.dat", "wb");
    if (f != NULL) {
        fwrite(&nb_transactions, sizeof(int), 1, f);
        fwrite(&next_transaction_id, sizeof(int), 1, f);
        if (nb_transactions > 0) {
            fwrite(transactions, sizeof(Transaction), nb_transactions, f);
        }
        fclose(f);
    }

    /* Export texte discret */
    exporter_tout_txt();
}

/* Chargement silencieux des donnees depuis "database/*.dat" */
void charger_donnees() {
    FILE *f;

    /* Clients */
    f = fopen(DB_DIR "/clients.dat", "rb");
    if (f != NULL) {
        fread(&nb_clients, sizeof(int), 1, f);
        if (nb_clients > 0) {
            clients = malloc(nb_clients * sizeof(Client));
            fread(clients, sizeof(Client), nb_clients, f);
        }
        fclose(f);
    } else {
        nb_clients = 0;
    }

    /* Comptes */
    f = fopen(DB_DIR "/comptes.dat", "rb");
    if (f != NULL) {
        fread(&nb_comptes, sizeof(int), 1, f);
        if (nb_comptes > 0) {
            comptes = malloc(nb_comptes * sizeof(Compte));
            fread(comptes, sizeof(Compte), nb_comptes, f);
        }
        fclose(f);
    } else {
        nb_comptes = 0;
    }

    /* Transactions */
    f = fopen(DB_DIR "/transactions.dat", "rb");
    if (f != NULL) {
        fread(&nb_transactions, sizeof(int), 1, f);
        fread(&next_transaction_id, sizeof(int), 1, f);
        if (nb_transactions > 0) {
            transactions = malloc(nb_transactions * sizeof(Transaction));
            fread(transactions, sizeof(Transaction), nb_transactions, f);
        }
        fclose(f);
    } else {
        nb_transactions = 0;
        next_transaction_id = 1;
    }
}

/* ========== ADMIN FUNCTIONS ========== */

int authentifier_admin() {
    char code[100];
    printf("\n=======================================\n");
    printf("||      ACCES ADMINISTRATEUR          ||\n");
    printf("=======================================\n");
    printf("Entrez le code d'acces admin: ");

    if (fgets(code, sizeof(code), stdin) != NULL) {
        code[strcspn(code, "\n")] = 0; /* Supprimer le saut de ligne */

        if (strcmp(code, "benmsik_bank_admin_access") == 0) {
            printf("\n [OK] Acces autorise\n");
            pause_screen();
            return 1;
        } else {
            printf("\n [ERREUR] Code d'acces invalide\n");
            return 0;
        }
    }
    return 0;
}

void informations_systeme() {
    printf("\n=======================================\n");
    printf("||      INFORMATIONS SYSTEME          ||\n");
    printf("=======================================\n");
    printf("Nombre total de clients: %d\n", nb_clients);
    printf("Nombre total de comptes: %d\n", nb_comptes);
    printf("Nombre total de transactions: %d\n", nb_transactions);
    printf("Prochain ID transaction: %d\n", next_transaction_id);

    /* Calculer le solde total */
    float solde_total = 0;
    for (int i = 0; i < nb_comptes; i++) {
        solde_total += comptes[i].solde;
    }
    printf("Solde total de tous les comptes: %.2f DH\n", solde_total);

    printf("\nFichiers de donnees:\n");
    printf("- database/clients.dat\n");
    printf("- database/comptes.dat\n");
    printf("- database/transactions.dat\n");
    printf("- database/backdoor acces/*.txt\n");

    pause_screen();
}

void afficher_statistiques() {
    printf("\n=======================================\n");
    printf("||         STATISTIQUES              ||\n");
    printf("=======================================\n");

    printf("\n--- CLIENTS ---\n");
    printf("Total: %d clients\n", nb_clients);

    /* Clients avec comptes */
    int clients_avec_comptes = 0;
    for (int i = 0; i < nb_clients; i++) {
        if (compter_comptes_client(clients[i].id_client) > 0) {
            clients_avec_comptes++;
        }
    }
    printf("Clients avec comptes: %d\n", clients_avec_comptes);
    printf("Clients sans comptes: %d\n", nb_clients - clients_avec_comptes);

    printf("\n--- COMPTES ---\n");
    printf("Total: %d comptes\n", nb_comptes);

    /* Calculer statistiques des comptes */
    float solde_total = 0, solde_min = 0, solde_max = 0;
    if (nb_comptes > 0) {
        solde_min = comptes[0].solde;
        solde_max = comptes[0].solde;
        for (int i = 0; i < nb_comptes; i++) {
            solde_total += comptes[i].solde;
            if (comptes[i].solde < solde_min) solde_min = comptes[i].solde;
            if (comptes[i].solde > solde_max) solde_max = comptes[i].solde;
        }
        printf("Solde total: %.2f DH\n", solde_total);
        printf("Solde moyen: %.2f DH\n", solde_total / nb_comptes);
        printf("Solde minimum: %.2f DH\n", solde_min);
        printf("Solde maximum: %.2f DH\n", solde_max);
    }

    printf("\n--- TRANSACTIONS ---\n");
    printf("Total: %d transactions\n", nb_transactions);

    /* Statistiques par type */
    int versements = 0, retraits = 0, virements = 0;
    float montant_versements = 0, montant_retraits = 0, montant_virements = 0;

    for (int i = 0; i < nb_transactions; i++) {
        if (strcmp(transactions[i].type, "Versement") == 0) {
            versements++;
            montant_versements += transactions[i].montant;
        } else if (strcmp(transactions[i].type, "Retrait") == 0) {
            retraits++;
            montant_retraits += transactions[i].montant;
        } else if (strcmp(transactions[i].type, "Virement") == 0) {
            virements++;
            montant_virements += transactions[i].montant;
        }
    }

    printf("Versements: %d (Total: %.2f DH)\n", versements, montant_versements);
    printf("Retraits: %d (Total: %.2f DH)\n", retraits, montant_retraits);
    printf("Virements: %d (Total: %.2f DH)\n", virements, montant_virements);

    printf("\n=======================================\n");
    pause_screen();
}

void afficher_tous_clients_detaille() {
    printf("\n=======================================\n");
    printf("||    TOUS LES CLIENTS (DETAILLE)     ||\n");
    printf("=======================================\n");

    if (nb_clients == 0) {
        printf("\n Aucun client enregistre\n");
        pause_screen();
        return;
    }

    for (int i = 0; i < nb_clients; i++) {
        printf("\n--- Client #%d ---\n", i + 1);
        printf("ID Client: %d\n", clients[i].id_client);
        printf("Nom: %s\n", clients[i].nom);
        printf("Prenom: %s\n", clients[i].prenom);
        printf("Profession: %s\n", clients[i].profession);
        printf("Telephone: %s\n", clients[i].num_tel);
        printf("Nombre de comptes: %d\n", compter_comptes_client(clients[i].id_client));
        printf("--------------------------------------\n");
    }

    pause_screen();
}

void afficher_tous_comptes_detaille() {
    printf("\n=======================================\n");
    printf("||    TOUS LES COMPTES (DETAILLE)     ||\n");
    printf("=======================================\n");

    if (nb_comptes == 0) {
        printf("\n Aucun compte enregistre\n");
        pause_screen();
        return;
    }

    for (int i = 0; i < nb_comptes; i++) {
        int idx_client = chercherClientParId(comptes[i].id_client);
        printf("\n--- Compte #%d ---\n", i + 1);
        printf("ID Compte: %d\n", comptes[i].id_compte);
        printf("Client: %s %s (ID: %d)\n",
               idx_client != -1 ? clients[idx_client].nom : "Inconnu",
               idx_client != -1 ? clients[idx_client].prenom : "",
               comptes[i].id_client);
        printf("Solde: %.2f DH\n", comptes[i].solde);
        printf("Date ouverture: %s\n", comptes[i].date_ouverture);
        printf("PIN: **** (masque)\n");
        printf("Nombre de transactions: ");

        /* Compter les transactions pour ce compte */
        int nb_trans = 0;
        for (int j = 0; j < nb_transactions; j++) {
            if (transactions[j].id_compte == comptes[i].id_compte) {
                nb_trans++;
            }
        }
        printf("%d\n", nb_trans);
        printf("--------------------------------------\n");
    }

    pause_screen();
}

void afficher_toutes_transactions_detaille() {
    printf("\n=======================================\n");
    printf("||  TOUTES LES TRANSACTIONS (DETAILLE) ||\n");
    printf("=======================================\n");

    if (nb_transactions == 0) {
        printf("\n Aucune transaction enregistree\n");
        pause_screen();
        return;
    }

    for (int i = 0; i < nb_transactions; i++) {
        printf("\n--- Transaction #%d ---\n", i + 1);
        afficher_une_transaction(&transactions[i]);
        printf("\n");
    }

    pause_screen();
}

void afficher_client_complet(int id_client) {
    int idx = chercherClientParId(id_client);
    if (idx == -1) {
        printf("\n [ERREUR] Client introuvable\n");
        pause_screen();
        return;
    }

    printf("\n=======================================\n");
    printf("||      FICHE CLIENT COMPLETE         ||\n");
    printf("=======================================\n");
    printf("ID Client: %d\n", clients[idx].id_client);
    printf("Nom: %s\n", clients[idx].nom);
    printf("Prenom: %s\n", clients[idx].prenom);
    printf("Profession: %s\n", clients[idx].profession);
    printf("Telephone: %s\n", clients[idx].num_tel);

    /* Afficher tous les comptes du client */
    printf("\n--- COMPTES DU CLIENT ---\n");
    int nb_comptes_cl = 0;
    float solde_total = 0;

    for (int i = 0; i < nb_comptes; i++) {
        if (comptes[i].id_client == id_client) {
            nb_comptes_cl++;
            solde_total += comptes[i].solde;
            printf("\nCompte #%d:\n", nb_comptes_cl);
            printf("  ID Compte: %d\n", comptes[i].id_compte);
            printf("  Solde: %.2f DH\n", comptes[i].solde);
            printf("  Date ouverture: %s\n", comptes[i].date_ouverture);
        }
    }

    if (nb_comptes_cl == 0) {
        printf("Aucun compte\n");
    } else {
        printf("\nTotal comptes: %d\n", nb_comptes_cl);
        printf("Solde total: %.2f DH\n", solde_total);
    }

    pause_screen();
}

void afficher_compte_complet(int id_compte) {
    int idx = chercherCompteParId(id_compte);
    if (idx == -1) {
        printf("\n [ERREUR] Compte introuvable\n");
        pause_screen();
        return;
    }

    int idx_client = chercherClientParId(comptes[idx].id_client);

    printf("\n=======================================\n");
    printf("||      FICHE COMPTE COMPLETE         ||\n");
    printf("=======================================\n");
    printf("ID Compte: %d\n", comptes[idx].id_compte);
    printf("Client: %s %s (ID: %d)\n",
           idx_client != -1 ? clients[idx_client].nom : "Inconnu",
           idx_client != -1 ? clients[idx_client].prenom : "",
           comptes[idx].id_client);
    printf("Solde actuel: %.2f DH\n", comptes[idx].solde);
    printf("Date ouverture: %s\n", comptes[idx].date_ouverture);
    printf("PIN: %s (ADMIN - affichage autorise)\n", comptes[idx].pin);

    /* Afficher toutes les transactions */
    printf("\n--- HISTORIQUE COMPLET ---\n");
    int found = 0;
    for (int i = 0; i < nb_transactions; i++) {
        if (transactions[i].id_compte == id_compte) {
            found = 1;
            afficher_une_transaction(&transactions[i]);
            printf("\n");
        }
    }

    if (!found) {
        printf("Aucune transaction\n");
    }

    pause_screen();
}

void menu_admin() {
    int choix;
    do {
        printf("\n");
        printf("=======================================\n");
        printf("||      MENU ADMINISTRATEUR          ||\n");
        printf("=======================================\n");
        printf("||  1. Statistiques du systeme      ||\n");
        printf("||  2. Voir tous les clients        ||\n");
        printf("||  3. Voir tous les comptes        ||\n");
        printf("||  4. Voir toutes les transactions ||\n");
        printf("||  5. Fiche client complete        ||\n");
        printf("||  6. Fiche compte complete        ||\n");
        printf("||  7. Informations systeme         ||\n");
        printf("||  8. Retour au menu principal     ||\n");
        printf("=======================================\n");
        printf("Votre choix: ");
        scanf("%d", &choix);
        clear_buffer();

        switch(choix) {
            case 1: afficher_statistiques(); break;
            case 2: afficher_tous_clients_detaille(); break;
            case 3: afficher_tous_comptes_detaille(); break;
            case 4: afficher_toutes_transactions_detaille(); break;
            case 5: {
                int id = saisir_int("ID client: ");
                afficher_client_complet(id);
                break;
            }
            case 6: {
                int id = saisir_int("ID compte: ");
                afficher_compte_complet(id);
                break;
            }
            case 7: informations_systeme(); break;
            case 8:
                printf("\n Retour au menu principal...\n");
                break;
            default:
                printf("\n [ERREUR] Choix invalide\n");
                pause_screen();
        }
    } while (choix != 8);
}

/* ========== MENUS ========== */

void Gestion_des_clients() {
    int choix;
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
        scanf("%d", &choix);
        clear_buffer();

        switch(choix) {
            case 1: Ajoute_un_client(); break;
            case 2: Modifications(); break;
            case 3: Suppression(); break;
            case 4: Recherche(); break;
            case 5: break;
            default: printf("\n [ERREUR] Choix invalide\n"); pause_screen();
        }
    } while (choix != 5);
}

void Gestion_des_comptes() {
    int choix;
    do {
        printf("\n**************************************\n");
        printf("*       GESTION DES COMPTES          *\n");
        printf("**************************************\n");
        printf("*   1. Creer un nouveau compte       *\n");
        printf("*   2. Consulter les comptes         *\n");
        printf("*   3. Fermer un compte              *\n");
        printf("*   4. Retour au menu principal      *\n");
        printf("**************************************\n");
        printf("Votre choix: ");
        scanf("%d", &choix);
        clear_buffer();

        switch(choix) {
            case 1: Nouveau_compte(); break;
            case 2: consultation(); break;
            case 3: fermeture_du_compte(); break;
            case 4: break;
            default: printf("\n [ERREUR] Choix invalide\n"); pause_screen();
        }
    } while (choix != 4);
}

void Gestion_des_operation() {
    int choix;
    do {
        printf("\n**************************************\n");
        printf("*      GESTION DES OPERATIONS        *\n");
        printf("**************************************\n");
        printf("*   1. Versement                     *\n");
        printf("*   2. Retrait                       *\n");
        printf("*   3. Virement                      *\n");
        printf("*   4. Historique des transactions   *\n");
        printf("*   5. Retour au menu principal      *\n");
        printf("**************************************\n");
        printf("Votre choix: ");
        scanf("%d", &choix);
        clear_buffer();

        switch(choix) {
            case 1: Versement(); break;
            case 2: Retrait(); break;
            case 3: Virement(); break;
            case 4: afficher_historique(); break;
            case 5: break;
            default: printf("\n [ERREUR] Choix invalide\n"); pause_screen();
        }
    } while (choix != 5);
}

/* Menu principal : redirige vers les 3 sous-menus ou quitte */
void menu_principale() {
    int choix;
    char input[100];
    do {
        printf("\n");
        printf("=======================================\n");
        printf("||    SYSTEME DE GESTION BANCAIRE    ||\n");
        printf("=======================================\n");
        printf("||          MENU PRINCIPAL           ||\n");
        printf("=======================================\n");
        printf("||  1. Gestion des clients           ||\n");
        printf("||  2. Gestion des comptes           ||\n");
        printf("||  3. Gestion des operations        ||\n");
        printf("||  4. Quitter                       ||\n");
        printf("=======================================\n");
        printf("Votre choix: ");

        /* Lire l'entrée comme chaîne pour détecter le code secret */
        if (fgets(input, sizeof(input), stdin) != NULL) {
            /* Supprimer le saut de ligne */
            input[strcspn(input, "\n")] = 0;

            /* Vérifier le code secret admin */
            if (strcmp(input, "#admin_bypass_system") == 0) {
                if (authentifier_admin()) {
                    menu_admin();
                    continue;
                } else {
                    pause_screen();
                    continue;
                }
            }

            /* Essayer de convertir en entier */
            if (sscanf(input, "%d", &choix) != 1) {
                printf("\n [ERREUR] Choix invalide\n");
                pause_screen();
                continue;
            }
        } else {
            choix = 4; /* Quitter en cas d'erreur */
        }

        switch(choix) {
            case 1: Gestion_des_clients(); break;
            case 2: Gestion_des_comptes(); break;
            case 3: Gestion_des_operation(); break;
            case 4:
                printf("\n Au revoir!\n");
                break;
            default:
                printf("\n [ERREUR] Choix invalide\n");
                pause_screen();
        }
    } while (choix != 4);
}

/* ========== LANCEMENT GLOBAL ========== */

void run_application() {
    printf("\n=======================================\n");
    printf("     BanquE Ben M´sik                      \n");
    printf("=======================================\n");

    charger_donnees();      /* Chargement silencieux */
    menu_principale();      /* Menus et interactions */
    sauvegarder_donnees();  /* Sauvegarde automatique silencieuse */

    if (clients != NULL) free(clients);
    if (comptes != NULL) free(comptes);
    if (transactions != NULL) free(transactions);
}

/* ========== MAIN MINIMALISTE ========== */

int main() {
    run_application();
    return 0;
}
