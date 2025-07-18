#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include "mikroC.h"

#define prvni z.z.prvni
#define druhy z.z.druhy
#define treti z.z.treti
#define ctvrty z.z.ctvrty

Uzel *Koren = nullptr;
std::map<std::string, int> Pamet;

Uzel *GenUzel(int typ, Uzel *a, Uzel *b, Uzel *c, Uzel *d)
{
    Uzel *u = new Uzel;
    u->Typ = typ;
    u->prvni = a;
    u->druhy = b;
    u->treti = c;
    u->ctvrty = d;
    return u;
}

Uzel *GenCislo(int c)
{
    Uzel *u = new Uzel;
    u->Typ = CISLO;
    u->z.Cislo = c;
    return u;
}

Uzel *GenRetez(const char *r)
{
    Uzel *u = new Uzel;
    u->Typ = RETEZ;
    u->z.Retez = strdup(r);
    return u;
}

Uzel *GenPromen(const char *r)
{
    Uzel *u = new Uzel;
    u->Typ = PROMENNA;
    u->z.Retez = strdup(r);
    return u;
}

int Vypocet(Uzel *u);

static int VypocetBin(Uzel *u, int (*op)(int, int))
{
    return op(Vypocet(u->prvni), Vypocet(u->druhy));
}

static int VypocetBinSafe(Uzel *u, int (*op)(int, int), const char *chyb_msg)
{
    int prava = Vypocet(u->druhy);
    if (prava == 0)
    {
        fprintf(stderr, "%s\n", chyb_msg);
        exit(1);
    }
    return op(Vypocet(u->prvni), prava);
}

static int ProvedPrirazeni(Uzel *u, bool zapis)
{
    std::string jmeno(u->prvni->z.Retez);
    int &cil = Pamet[jmeno];
    int prava = Vypocet(u->druhy);

    switch (u->Typ)
    {
    case P_PRICT:
        return zapis ? (cil += prava) : cil + prava;
    case P_ODECT:
        return zapis ? (cil -= prava) : cil - prava;
    case P_NASOB:
        return zapis ? (cil *= prava) : cil * prava;
    case P_DELEN:
    case P_MODUL:
        if (prava == 0)
        {
            fprintf(stderr, "Deleni nulou\n");
            exit(1);
        }
        return (u->Typ == P_DELEN) ? (zapis ? (cil /= prava) : cil / prava) : (zapis ? (cil %= prava) : cil % prava);
    case P_POSUNVLEVO:
        return zapis ? (cil <<= prava) : cil << prava;
    case P_POSUNVPRAVO:
        return zapis ? (cil >>= prava) : cil >> prava;
    case P_AND:
        return zapis ? (cil &= prava) : cil & prava;
    case P_OR:
        return zapis ? (cil |= prava) : cil | prava;
    case P_XOR:
        return zapis ? (cil ^= prava) : cil ^ prava;
    case '=':
        return zapis ? (cil = prava) : prava;
    default:
        fprintf(stderr, "Neznamy typ prirazeni: %d\n", u->Typ);
        exit(1);
    }
}

int Vypocet(Uzel *u)
{
    if (!u)
        return 0;

    switch (u->Typ)
    {
    case 0:
        Vypocet(u->prvni);
        Vypocet(u->druhy);
        return 0;
    case CISLO:
        return u->z.Cislo;
    case PROMENNA:
        return Pamet[std::string(u->z.Retez)];

    case '+':
    case PLUS:
        return VypocetBin(u, [](int a, int b)
                          { return a + b; });
    case '-':
    case MINUS:
        return VypocetBin(u, [](int a, int b)
                          { return a - b; });
    case '*':
        return VypocetBin(u, [](int a, int b)
                          { return a * b; });
    case '/':
        return VypocetBinSafe(u, [](int a, int b)
                              { return a / b; }, "Deleni nulou");
    case '%':
        return VypocetBinSafe(u, [](int a, int b)
                              { return a % b; }, "Deleni nulou");
    case '<':
        return VypocetBin(u, [](int a, int b) -> int
                          { return a < b; });
    case '>':
        return VypocetBin(u, [](int a, int b) -> int
                          { return a > b; });
    case MENSIROVNO:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a <= b; });
    case VETSIROVNO:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a >= b; });
    case ROVNO:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a == b; });
    case NENIROVNO:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a != b; });
    case AND:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a && b; });
    case OR:
        return VypocetBin(u, [](int a, int b) -> int
                          { return a || b; });
    case '!':
    case NOT:
        return !Vypocet(u->prvni);
    case '~':
        return ~Vypocet(u->prvni);
    case '&':
        return VypocetBin(u, [](int a, int b)
                          { return a & b; });
    case '|':
        return VypocetBin(u, [](int a, int b)
                          { return a | b; });
    case POSUNVLEVO:
        return VypocetBin(u, [](int a, int b)
                          { return a << b; });
    case POSUNVPRAVO:
        return VypocetBin(u, [](int a, int b)
                          { return a >> b; });

    case INKREM:
    {
        std::string jmeno(u->prvni->z.Retez);
        return ++Pamet[jmeno];
    }
    case DEKREM:
    {
        std::string jmeno(u->prvni->z.Retez);
        return --Pamet[jmeno];
    }

    case '=':
    case P_PRICT:
    case P_ODECT:
    case P_NASOB:
    case P_DELEN:
    case P_MODUL:
    case P_POSUNVLEVO:
    case P_POSUNVPRAVO:
    case P_AND:
    case P_OR:
    case P_XOR:
        return ProvedPrirazeni(u, true);

    default:
        fprintf(stderr, "Neznamy vyraz: %d [%c] (adresa: %p)\n", u->Typ, (char)u->Typ, (void *)u);
        exit(1);
    }
}

void Vykonej(Uzel *u)
{
    if (!u)
        return;

    switch (u->Typ)
    {
    case 0:
    case ';':
        Vykonej(u->prvni);
        Vykonej(u->druhy);
        break;
    case '=':
    case P_PRICT:
    case P_ODECT:
    case P_NASOB:
    case P_DELEN:
    case P_MODUL:
    case P_POSUNVLEVO:
    case P_POSUNVPRAVO:
    case P_AND:
    case P_OR:
    case P_XOR:
        ProvedPrirazeni(u, true);
        break;
    case PRINT:
        if (u->druhy)
            printf(u->prvni->z.Retez, Vypocet(u->druhy));
        else
            printf("%s", u->prvni->z.Retez);
        break;
    case SCAN:
    {
        std::string jmeno(u->prvni->z.Retez);
        int hodnota;
        scanf("%d", &hodnota);
        Pamet[jmeno] = hodnota;
        break;
    }
    case IF:
        if (Vypocet(u->prvni))
            Vykonej(u->druhy);
        else if (u->treti)
            Vykonej(u->treti);
        break;
    case WHILE:
        while (Vypocet(u->prvni))
            Vykonej(u->druhy);
        break;
    case DO:
        do
        {
            Vykonej(u->prvni);
        } while (Vypocet(u->druhy));
        break;
    case FOR:
        Vykonej(u->prvni);
        while (Vypocet(u->druhy))
        {
            Vykonej(u->ctvrty);
            Vykonej(u->treti);
        }
        break;
    default:
        Vypocet(u);
    }
}

void UvolniUzel(Uzel *u)
{
    if (!u)
        return;

    switch (u->Typ)
    {
    case CISLO:
        break;
    case PROMENNA:
    case RETEZ:
        free((void *)u->z.Retez);
        break;
    default:
        UvolniUzel(u->prvni);
        UvolniUzel(u->druhy);
        UvolniUzel(u->treti);
        UvolniUzel(u->ctvrty);
    }

    delete u;
}

void Chyba(const char *zprava, Pozice poz)
{
    extern unsigned int Radek;
    extern unsigned int Sloupec;

    switch (poz)
    {
    case RADEK:
        fprintf(stderr, "%d %s\n", Radek, zprava);
        break;
    case SLOUPEC:
        fprintf(stderr, "%d.%d %s\n", Radek, Sloupec, zprava);
        break;
    default:
        fprintf(stderr, "Chyba (bez pozice): %s\n", zprava);
        break;
    }
}

void yyerror(const char *s)
{
    Chyba(s, SLOUPEC);
}

extern int yyparse();
extern void LexInit();

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Pouziti: %s <soubor>\n", argv[0]);
        return 1;
    }

    FILE *soubor = fopen(argv[1], "r");
    if (!soubor)
    {
        perror("Chyba pri otevirani souboru");
        return 1;
    }

    extern FILE *yyin;
    yyin = soubor;

    LexInit();
    if (yyparse() == 0 && Koren)
    {
        Vykonej(Koren);
        UvolniUzel(Koren);
    }

    fclose(soubor);
    return 0;
}
