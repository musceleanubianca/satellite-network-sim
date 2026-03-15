#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct nod
{
    int frecv; // frecventa totala pentru nodurile Huffman
    char nume[2048];
    struct nod *st, *dr;
} frunza, *arbore;

typedef struct heap // structura de min-Heap pentru arborele Huffman
{
    int memorie, nrElem;
    frunza **v;
} Heap;

int comparare(frunza *a, frunza *b) // functia de comparare
{
    if (a->frecv != b->frecv)
        return a->frecv - b->frecv;
    return strcmp(a->nume, b->nume);
}

void swap(frunza **a, frunza **b)
{
    frunza *tmp = *a;
    *a = *b;
    *b = tmp;
}

Heap *initHeap(int capacitate) // initializarea Heap-ului
{
    Heap *h = (Heap *)malloc(sizeof(Heap));
    h->memorie = capacitate;
    h->nrElem = 0;
    h->v = (frunza **)malloc(sizeof(frunza *) * capacitate);
    return h;
}

void insertHeap(Heap *h, frunza *elem) // insereaza noduri in Heap
{
    if (h->nrElem == h->memorie)
    {
        h->memorie *= 2;
        h->v = realloc(h->v, h->memorie * sizeof(frunza *));
    }
    int i = h->nrElem++;
    h->v[i] = elem;
    while (i > 0 && comparare(h->v[i], h->v[(i - 1) / 2]) < 0)
    {
        swap(&h->v[i], &h->v[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

frunza *extractHeap(Heap *h) // extrage nodul minim
{
    frunza *min = h->v[0];
    h->v[0] = h->v[--h->nrElem];

    int i = 0;
    while (1)
    {
        int st = 2 * i + 1;
        int dr = 2 * i + 2;
        int element = i;

        if (st < h->nrElem && comparare(h->v[st], h->v[element]) < 0)
            element = st;
        if (dr < h->nrElem && comparare(h->v[dr], h->v[element]) < 0)
            element = dr;

        if (element != i)
        {
            swap(&h->v[i], &h->v[element]);
            i = element;
        }
        else
            break;
    }

    return min;
}

void freeHeap(Heap **h) // eliberare de memorie pentru Heap
{
    free((*h)->v);
    free(*h);
    *h = NULL;
}

arbore creare(int frecv, const char *nume) // crearea unui nod nou in arbore
{
    arbore nou = (arbore)malloc(sizeof(frunza));
    nou->frecv = frecv;
    strncpy(nou->nume, nume, sizeof(nou->nume) - 1);
    nou->nume[sizeof(nou->nume) - 1] = '\0';
    nou->st = nou->dr = NULL;
    return nou;
}

void freeArbore(arbore r) // eliberare de memorie pentru arborele Huffman
{
    if (!r)
        return;
    freeArbore(r->st);
    freeArbore(r->dr);
    free(r);
}

arbore buildTree(int N, int frecvente[], char numeSatelit[][2048]) // crearea arborelui Huffman
{
    Heap *h = initHeap(N);
    for (int i = 0; i < N; i++)
    {
        arbore nou = creare(frecvente[i], numeSatelit[i]);
        insertHeap(h, nou);
    }

    while (h->nrElem > 1) // construiesc arborele dupa noduri cu frecvente minime
    {
        arbore a = extractHeap(h);
        arbore b = extractHeap(h);
        arbore parinte = creare(a->frecv + b->frecv, ""); // nod parinte cu frecventa suma celor doua noduri

        if (comparare(a, b) < 0) // legam nodurile
        {
            parinte->st = a;
            parinte->dr = b;
        }
        else
        {
            parinte->st = b;
            parinte->dr = a;
        }

        strncpy(parinte->nume, parinte->st->nume, sizeof(parinte->nume) - 1);
        parinte->nume[sizeof(parinte->nume) - 1] = '\0';

        strncat(parinte->nume, parinte->dr->nume, sizeof(parinte->nume) - strlen(parinte->nume) - 1); // numele parintelui este concatenarea numelor fiilor

        insertHeap(h, parinte);
    }

    arbore radacina = extractHeap(h);
    freeHeap(&h);
    return radacina;
}

void printArbore(arbore radacina, FILE *fout)
{
    arbore coada[1000]; // parcurgem cu o coada pentru a afisa pe niveluri
    int start = 0, end = 0;
    coada[end++] = radacina;

    while (start < end)
    {
        int nivel = end - start; // calculez cate noduri sunt pe nivelul curent
        for (int i = 0; i < nivel; i++)
        {
            arbore curent = coada[start++];
            fprintf(fout, "%d-%s", curent->frecv, curent->nume); // afisez frecventa si numele nodului curent
            if (i < nivel - 1)
                fprintf(fout, " "); // pun spatiu daca nu e ultimul nod de pe nivel
            // Daca exista, adaug fiii in coada
            if (curent->st)
                coada[end++] = curent->st;
            if (curent->dr)
                coada[end++] = curent->dr;
        }
        fprintf(fout, "\n");
    }
}

void decodificareMesaj(arbore radacina, const char *cod, FILE *fout)
{
    arbore curent = radacina;

    for (int j = 0; cod[j] == '0' || cod[j] == '1'; j++) // parcurgerea codificarilor
    {
        if (cod[j] == '0')
            curent = curent->st; // daca e 0, merg la stanga
        else if (cod[j] == '1')
            curent = curent->dr; // daca e 1, merg la dreapta
        if (!curent)
            break;

        if (!curent->st && !curent->dr) // daca ajung la o frunza...
        {
            fprintf(fout, "%s", curent->nume); //...afisez numele ei
            if (cod[j + 1] == '0' || cod[j + 1] == '1')
                fprintf(fout, " "); // afisez spatiu daca mai urmeaza biti
            curent = radacina;
        }
    }

    fprintf(fout, "\n");
}

void generareCoduri(arbore rad, char cod_curent[], char coduri[][100], char numeSatelit[][16], int *index)
{
    if (!rad->st && !rad->dr) // daca sunt la o frunza...
    {
        strcpy(numeSatelit[*index], rad->nume); //...salvez numele frunzei
        strcpy(coduri[*index], cod_curent);     //...salvez codul frunzei
        (*index)++;                             // trec la urmatoarea frunza
        return;
    }

    // Adaug 0 la cod si trec in subarborele stang
    int len = strlen(cod_curent);
    cod_curent[len] = '0';
    cod_curent[len + 1] = '\0';
    generareCoduri(rad->st, cod_curent, coduri, numeSatelit, index);

    // Inlocuiesc cu 1 si trec in subarborele drept
    cod_curent[len] = '1';
    cod_curent[len + 1] = '\0';
    generareCoduri(rad->dr, cod_curent, coduri, numeSatelit, index);

    // Revin la starea precedenta
    cod_curent[len] = '\0';
}

arbore gasireParinte(arbore radacina, char satelit[][16], int nrSat) // cel mai apropiat parinte comun
{
    if (!radacina)
        return NULL;
    for (int i = 0; i < nrSat; i++) // verificare daca nodul actual e cel cautat
    {
        if (strcmp(radacina->nume, satelit[i]) == 0)
            return radacina; // returnez nodul daca e cel cautat
    }
    // caut in subarborele stang, respectiv drept
    arbore st = gasireParinte(radacina->st, satelit, nrSat);
    arbore dr = gasireParinte(radacina->dr, satelit, nrSat);

    if (st && dr) // daca gasesc in ambii subarbori, returnez nodul curent
        return radacina;

    if (st) // daca gasesc doar in stanga, returnez nodul de acolo
        return st;
    else
        return dr; // analog, returnez ce e in dreapta
}

void cerinta1(FILE *fin, FILE *fout)
{
    int N;
    fscanf(fin, "%d", &N); // citesc nr de sateliti
    int frecv[N];
    char nume[N][2048];

    for (int i = 0; i < N; i++)
        fscanf(fin, "%d %s", &frecv[i], nume[i]); // citesc frecevnta si numele

    arbore radacina = buildTree(N, frecv, nume); // construiesc arborele Huffman pe baza frecventelor si numelor
    if (!radacina)
        return;
    printArbore(radacina, fout); // afisare pe niveluri
    freeArbore(radacina);
}

void cerinta2(FILE *fin, FILE *fout)
{
    int N;
    if (fscanf(fin, "%d", &N) != 1 || N <= 0)
        return; // verificare citire

    int frecv[N];
    char nume[N][2048];

    for (int i = 0; i < N; i++)
    {
        if (fscanf(fin, "%d %15s", &frecv[i], nume[i]) != 2)
            return; // verific daca s-au citit corect numele si frecventa
    }

    arbore radacina = buildTree(N, frecv, nume);
    if (!radacina)
        return;

    int nrLinii;
    if (fscanf(fin, "%d\n", &nrLinii) != 1 || nrLinii <= 0)
    {
        freeArbore(radacina);
        return; // citire si validare linii de decodificat
    }

    char cod[1024];
    for (int i = 0; i < nrLinii; i++)
    {
        if (!fgets(cod, sizeof(cod), fin))
            break;                              // citirea codurilor de pe linii
        decodificareMesaj(radacina, cod, fout); // decodificarea mesajelor
    }

    freeArbore(radacina);
}

void cerinta3(FILE *fin, FILE *fout)
{
    int N; // numar total de sateliti
    fscanf(fin, "%d", &N);

    int frecv[N];
    char nume[N][2048];

    for (int i = 0; i < N; i++)
    {
        fscanf(fin, "%d %15s", &frecv[i], nume[i]);
    }

    arbore rad = buildTree(N, frecv, nume);

    char coduri[N][100];
    char frunze[N][16];
    int index = 0;
    char cod_curent[100] = "";

    generareCoduri(rad, cod_curent, coduri, frunze, &index);

    int NC; // numarul satelitilor ce trebuie codificati
    fscanf(fin, "%d", &NC);
    for (int i = 0; i < NC; i++)
    {
        char satelit[16];
        fscanf(fin, "%15s", satelit);

        for (int j = 0; j < index; j++) // cautare si afisare cod
        {
            if (strcmp(frunze[j], satelit) == 0)
            {
                fprintf(fout, "%s", coduri[j]);
                if (i < NC - 1)
                    fprintf(fout, " "); // spatiu intre coduri
                break;
            }
        }
    }

    fprintf(fout, "\n");
    freeArbore(rad);
}

void cerinta4(FILE *fin, FILE *fout)
{
    int N;
    fscanf(fin, "%d", &N);
    int frecv[N];
    char nume[N][2048];

    for (int i = 0; i < N; i++)
        fscanf(fin, "%d %15s", &frecv[i], nume[i]);

    arbore rad = buildTree(N, frecv, nume);

    int PS; // numar perechi de sateliti
    fscanf(fin, "%d", &PS);

    char sateliti[PS][16];
    for (int i = 0; i < PS; i++)
        fscanf(fin, "%15s", sateliti[i]);

    arbore parinte = gasireParinte(rad, sateliti, PS); // cel mai apropiat parinte comun
    if (parinte)
        fprintf(fout, "%s\n", parinte->nume); // afiseaza numele
    freeArbore(rad);
}

int main(int argc, char *argv[])
{
    const char *optiune = argv[1];
    const char *input_file = argv[2];
    const char *output_file = argv[3];

    FILE *fin = fopen(input_file, "r");
    FILE *fout = fopen(output_file, "w");

    if (strcmp(optiune, "-c1") == 0)
        cerinta1(fin, fout);
    else if (strcmp(optiune, "-c2") == 0)
        cerinta2(fin, fout);
    else if (strcmp(optiune, "-c3") == 0)
        cerinta3(fin, fout);
    else if (strcmp(optiune, "-c4") == 0)
        cerinta4(fin, fout);

    fclose(fin);
    fclose(fout);
    return 0;
}
