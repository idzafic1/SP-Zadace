#include <stdio.h>                                 // Funkcije ulaza izlaza
#include <stdlib.h>                                // 
#include <string.h>                                // Funkcije za rad sa nizovima znakova
#include <ctype.h>                                 // isalpha i još neke
#define FALSE 0                                    // Netačno
#define TRUE 1                                     // Tačno
// Prepoznaje da li je na trenutnoj poziciji  niz znakova i ako jeste dodijeli varijabli
// znak odgovarajući kod i pomjeri poziciju u liniji za dužinu simbola
#define TOKEN(T,Z) \
  if (!strncmp(cp,T,\
  sizeof(T)-1)) {znak=Z;\
  pozicija+=sizeof(T)-2;} else
                                                   // 
FILE *ulaz, *izlaz;                                // Ulazna i izlazna tekstualna datoteka
int pozicija, trenlabela,                          // pokazuje na trenutni zmak na ulazu i brojač labela
    dlin = 0, brojaclinija,                        // dužina zadnje linije i broj pročitanih linija teksta
    DodjelaKarakteru;                              // Da li je zadja operacija radila s nizom znakova
unsigned char znak;                                // Varijabla u kojoj se nalazi zadnji preuzeti simbol
char linija[256], aktfun[256];                     // Tekst linije sa ulaza i ime trenutno aktivne funkcije
void IzrazDodjeljivanja(void);                     // Referenca unaprijed za uzajamno rekurzivne funkcije
void Blok(void);                                   // Referenca unaprijed za uzajamno rekurzivne funkcije 
enum kat {LOK, GLOB, FUN};                         // Kategorija imena lokalna var, globalna var, funkcija
struct tabela {                                    // Element tabele simbola
  char * simbol;                                   // Pokazivač na tekst imena
  struct tabela * dalje;                           // Ulančana lista na sljedeći element
  enum kat kt;                                     // Kategorija simbola
} * zadnji = NULL;                                 // Lista se dodaje od početka, pa je zadnji eleemnt NULL
                                                   // 
void NovaLabela(char *Ime) {                       // Na prosljeđenu lokaciju generiše ime nove labele
  trenlabela++;                                    // Ime se pravi uvećanjem brojača labkela
  sprintf(Ime, "L%d", trenlabela);                 // I ispred broja dodaje slovo L
}                                                  // 
                                                   // 
void Emit(char *st) {                              // Generisanje koda, proslijedi se asemblerska naredba
  fprintf(izlaz, "%s\r\n", st);                    // A ona se samo sa novim redom pošalje u datoteku
}                                                  // 
                                                   // 
void EmitL(char *st) {                             // Generisanje labele u kodu
  char lab[80];                                    // Ovdje se upiše tekst labele
  strcpy(lab, st);                                 // Iskopiraj prosljeđeno ime
  strcat(lab, ":");                                // Nadodaj mu dvotačku
  Emit(lab);                                       // Pošalji ime labele
}                                                  // 
                                                   // 
void Novi(void) {                                  // Čitanje novog simbola u varijablu "znak"
  char *cp, str1[256];                             // Pointer na trenutnu poziciju u linijij i pomoćni niz
  int p;                                           // Ne treba nam?
  do {                                             // Početak dohvatanja znaka
    if (feof(ulaz))                                // Ako je kraj ulazne datoteke
      break;                                       // iskoči
    pozicija++;                                    // Pređi na sljedeći simbol u liniji
    if (pozicija >= dlin)  {                       // Došli smo na kraj linije?
      brojaclinija++;                              // U tom slučaju, uvećaj brojać linija
      fgets(linija, 256, ulaz);                    // Pročitaj sa ulaza novu liniju
      dlin = strlen(linija);                       // Zapamti dužinu trenutne linije
      sprintf(str1, ";%s", linija);                // Pripremi izvornu liniju kao asemblerski komentar
      Emit(str1);                                  // Upiši u asemblerski listing
      pozicija = 0;                                // Idi na početak linije
    }                                              //
    cp = linija + pozicija;                        // Postavi pokazivač na trenutnu poziciju
    if (*linija != '\0')                           // Ako nismo na kraju linije
      znak = linija[pozicija];                     // znak je onaj simbol na poziciji
    else                                           // Ako jesmo na kraju linije
      znak = ' ';                                  // znak je blanko
    TOKEN(":=", 129)                               // Ako je na trenutnoj poziciji :=, znak će biti 128
    TOKEN("ako", 130)                              // Ako je na trenutnoj poziciji riječ "ako", znak=130
    TOKEN("dok", 131)                              // Ako je na trenutnoj poziciji riječ "dok", znak=131
    TOKEN("asembler", 132)                         // Ako je na tren. poziciji riječ "asembler", znak=132
    TOKEN("inace", 133)                            // Ako je na tren. poziciji riječ "inace", znak=133
    TOKEN("varijable", 134)                        // Ako je na tren. poziciji riječ "varijable", znak=134
    TOKEN("funkcija", 135)                         // Ako je na tren. poziciji riječ "funkcija", znak=135
    if (znak == '`')                               // Apostrof je komentar do kraja linije
      pozicija = dlin;                             // Ako je bio komentar idi na kraj linije
  } while(znak <= ' ' || znak == '`');             // Vrati se na dohvatanje znaka za blanko i komentar
}                                                  // 
                                                   // 
struct tabela * Unesen (char * sim) {              // Da li je simbol sa proslijeđenim imenom u tabelio
  struct tabela * iter;                            // Varijabla sadrži pointer na trenutni element tabele
  iter = zadnji;                                   // Pokaži zadnjeg elementa
  while (iter != NULL) {                           // Ako pokazujemo na prazno, kraj liste
    if (!strcmp(iter->simbol, sim))                // Ako je prosljeđeno ime jednako imenu u tabeli, 
      return iter;                                 //  tada vrati poziciju u tabeli simbola
    iter = iter->dalje;                            // Inače nastavi kroz ulančanu lisu
  }                                                // 
  return NULL;                                     // NULL znači da simbol da tim imenom nije nađen
}                                                  // 
                                                   // 
void DodajSimbol(char * sim, enum kat ka) {        // Ubaci simbol imena i kategorije u tabelu simbola
  int duz;                                         // Ovdje pamtimo dužinu imena simbola
  struct tabela * novitab;                         // Novi element ulančane liste tabele simbola
  char * novistr;                                  // Pokazuje na niz znakova koji ćemo alocirati
  duz = strlen(sim);                               // Uzmi dužinu proslijeđenog simbola
  novistr = malloc(duz + 1);                       // Alociraj za 1 bajt više zbog nule na kraju
  novitab = malloc(sizeof( struct tabela));        // Alociraj i element tabele simbola
  strcpy(novistr, sim);                            // Kopiraj niz znakova iz proslijeđenog u alociran
  novitab->simbol = novistr;                       // Postavi član elementa tabele na novi niz znakova
  novitab->dalje = zadnji;                         // Pokazivač na prethodni element, tj. zadnji dodan
  novitab->kt = ka;                                // Ubaci kategoriju
  zadnji = novitab;                                // Postavi zadnji dodani simbol u tabeli
}                                                  // 
                                                   // 
void Greska(char *poruka) {                        // Poziva se u slučaju greške, uz tekst poruke
  puts(poruka);                                    // Prikaži poruku
  fprintf(stderr, "%d:%s\n", brojaclinija, linija);// Prikaži liniju i mjesto greške
  if (ulaz != NULL)                                // Ako je otvorena ulazna datoteka
    fclose(ulaz);                                  // tada je zatvori
  if (izlaz != NULL)                               // Ako je otvorena izlazna datoteka
    fclose(izlaz);                                 // i nju zatvori
  exit(0);                                         // Napusti kompajler
}                                                  // 
                                                   // 
void MoraBiti(char z) {                            // Zahtijevaj da je proslijeđeni simbol na ulazu
  char str1[256];                                  // Ovdje tekst greške
  if (znak != z) {                                 // Ako na ulazu nije očekivani simbol
    sprintf(str1, "Ocekuje %c", z);                // Pripremi poruku o grešci
    Greska(str1);                                  // Priklaži poruku greške
  }                                                // 
  Novi();                                          // Bio je očekivani simbol, idi na naredni
}                                                  // 
                                                   // 
void UzmiIme(char *Ime) {                          // Dohvaća ime varijable/funkcije na datu adresu
  while (isalpha(znak)) {                          // Dok je god na ulazu veliko ili malo slovo
    *Ime++ = znak;                                 // Slovo upiši na kraj proslijeđenog niza
    Novi();                                        // Novo slovo
  }                                                // 
  *Ime = '\0';                                     // Na kraju niza simbol 0, terminator niza
}                                                  // 
                                                   // 
void StringKonstanta(void) {                       // Unutar dvostrukih navodnika nalazi se string 
  char str[256], lab[256];                         // Pomoćne varijable za string i labelu
  NovaLabela(lab);                                 // Generiši labelu koja će pamtiti adresu niza
  Emit("section .data");                           // Generiši prelaz u sekciju podataka
  EmitL(lab);                                      // Emituj labelui
  znak = linija[++pozicija];                       // Preskačemo dupli navodnik
  while (1) {                                      // U petlji
    if (pozicija >= dlin)                          // Ako smo prešli kraj linije
      Greska("Nema navodnika");                    // Greška jer je niz znakova nezatvoren
    znak = linija[pozicija++];                     // Znak ne uzimamo sa Novi, jer može biti blanko
    if (znak == '"')                               // Ali ako je dvostruki navodnik iskoči iz petlje
      break;                                       // 
    sprintf(str, " DB %d", znak);                  // Inače, generiši db sa ASCII kodom znaka
    Emit(str);                                     // I pošalji to na izlaz
  } ;                                              //
  pozicija--;                                      // Na kraju niza stavi poziciju nazad na navodnike
  Novi();                                          // Uzmi znak
  Emit(" DB 0");                                   // Generiši krajnju nulu
  Emit("section .text");                           // Vrati se su sekciju koda
  sprintf(str, " LEA EAX,[%s]", lab);              // Generiši da EAX pokazuje na niz znakova
  Emit(str);                                       // Emituj tu naredbu
}                                                  // 
                                                   // 
int UzmiKonstantu(void) {                          // Očitava sa ulaza cjelobrojnu konstantu
  int rezult = 0;                                  // Postavi rezultat na 0
  while (isdigit(znak)) {                          // Dokle god dolaze cifre
    rezult = rezult * 10 + znak - '0';             // znak-'0' pretvori ASCII u broj i dopiši cifru
    Novi();                                        // Uzmi sljedeću cifru
  }                                                // 
  return rezult;                                   // U varijabli rezult je konvertovan cijeli broj
}                                                  // 
                                                   // 
void EmitGetVal(int ChrIzraz) {                    // Generiše čitanje sa adrese pokazivane s EBX
  if (ChrIzraz) {                                  // Ako je operacija nad elementom niza znakova
    Emit( " MOVZX EAX,BYTE[EBX]");                 // Kupi se jedan bajt raširen u EAX
  } else                                           // U ostalim slučajevima
    Emit(" MOV EAX,[EBX]");                        // Uzimaju se četiri bajta u EAX
}                                                  // 
                                                   // 
void PokazivacINiz(int ChrIzraz) {                 // Element niza, proslijeđena vrsta
  Novi();                                          // Pomjeri se od otvorene zagrade
  Emit(" PUSH EAX");                               // U EAX je vrijednost - početna adresa niza
  IzrazDodjeljivanja();                            // Pozovi izraz za ndeks niza
  Emit(" POP EBX");                                // Vrati početnu adresu u EBX, EAX sadrži indeks
  if (!ChrIzraz)                                   // Za nizove cijelih brojeva
    Emit(" SAL EAX,2");                            // pomnoži indeks sa 4
  Emit(" ADD EBX,EAX");                            // Saberi adresu i skalirani indeks
  EmitGetVal(ChrIzraz);                            // Uzmi vrijednost elementa niza
  MoraBiti(']');                                   // Zatvori uglastu zagradu
}                                                  // 
                                                   // 
void PozivFunkcije(char * Ime) {                   // Poziv funkcije proslijeđenog imena
  int lokalnih = 0;                                // Brojač alociranog područja na steku za parametre
  char staddr[256];                                // Labela veličine alociranog područja
  char str1[256], str2[256];                       // Pomoćne varijable
  MoraBiti('(');                                   // Očekuj otvorenu zagradu
  NovaLabela(staddr);                              // Pripremi labelu
  sprintf(str1, " SUB ESP,%s", staddr);            // Umanjenje stek pointera za alocirano područje
  Emit(str1);                                      // Generiše se, ali još ga ne znamo
  while (znak != ')') {                            // Sve do zatvorene zagrade
    IzrazDodjeljivanja();                          // Generiši izraz: vrijednot u EAX, adresa u EBX
    sprintf(str2, " MOV [ESP+%d],EAX", lokalnih);  // Zbog StdCall redoslijeda ne idemo na PUSH EAX
    Emit(str2);                                    // nego MOV [ESP+lokalnih]
    lokalnih += 4;                                 // Prostor za naredni parametar
    if (znak != ')')                               // Ako nije zagrada
      MoraBiti(',');                               // onda očekuj zarez
  }                                                // 
  MoraBiti(')');                                   // Kraj parametara i zatvorena zagrada
  sprintf(str2, "%s EQU %d", staddr, lokalnih);    // Sada znamo koliko je alocirano područje za
  Emit(str2);                                      // parametre i EQU naredbu koja definiše labelu
  if (!Unesen(Ime))                                // Semantička provjera, da li je to funkcija
    Greska("Nema funkcije");                       // 
  sprintf(str2, " CALL %s", Ime);                  // Pripremi CALL naredbu i generiši je
  Emit(str2);                                      // Pozvana funkcija čisti stek (StdCall)
}                                                  // 
                                                   // 
void Varijabla(void) {                             // Emitovanje koda za vrijednost varijable 
  char Ime[256], znak1, str[256], str2[256];       // Zapamčeno ime i simbol, pomoćne varijable
  int chrIzraz;                                    // Niz znakova?
  chrIzraz = FALSE;                                // Inicijalno nije
  znak1 = znak;                                    // Zapamti simbol
  UzmiIme(Ime);                                    // Dohvati ime varijable
  if (znak == '(') {                               // Ako je nakon imena otvorena zagrada
    PozivFunkcije(Ime);                            // idi na poziv funkcije
    return;                                        //
  }                                                //
  sprintf(str2, "%s%s", Ime, aktfun);              // Spoji ime varijable i funkcije u ref. labelu
  if (Unesen(str2) )                               // Lokalne varijable su sa imenom funkcije
    sprintf(str,  " LEA EBX,[EBP+%s]", str2);      // i za njih se adresa računa relativno od EBP
  else if (Unesen(Ime))                            // Globalne varijable su sa imenom
    sprintf(str,  " LEA EBX,[%s]", Ime);           // i za njih se adresa računa apsolutno
  else                                             // Ako nema u tabeli simbola nijedne
    Greska("Nema varijable");                      // to je greška nedefinisane varijable
  Emit(str);                                       // Emitovaćemo naredbu uzimanja adrese
  chrIzraz = (znak1 == 'c');                       // Ako je prvo slovo imena 'c' to je pointer na char
  if (znak != '[') {                               // Ako nije element niza
    EmitGetVal(FALSE);                             // Pročitaj vrijednost kao četvorobajtni broj
    return;                                        // 
  }                                                //
  Emit(" MOV EAX,[EBX]");                          // Element niza, uzmi vrijednost sa adrese
  PokazivacINiz(chrIzraz);                         // Pozovi sabiranje sa indeksom
  if (znak == 129 && chrIzraz)                     // Ako slijedi := i pointer na char
    DodjelaKarakteru = TRUE;                       // Pripremi da slijedi dodjela znaklovnoj varijabli
}                                                  // 
                                                   //
void Faktor(void) {                                // Faktor obrađuje unarne operatore (prioritet 1)
  char str1[256];                                  // Pomoćna varijabla
  switch (znak) {                                  // Zavisno od simbola:
  case '-':                                        // Unarni minus
    Novi();                                        // Pređi na novi znak
    Faktor();                                      // Rekurzivni poziv, desni operator, rezultat u EAX
    Emit(" NEG EAX");                              // Generiši instrukciju NEG za promjenu predznaka
    break;                                         // 
  case '~':                                        // Logička negacija
    Novi();                                        // Pređi na novi znak
    Faktor();                                      // Rekurzivni poziv, desni operator, rezultat u EAX
    Emit(" NOT EAX");                              //
    break;                                         // 
  case '&':                                        // Operaor adrese
    Novi();                                        // Pređi na novi znak
    Faktor();                                      // Rekurzivni poziv, desni operator, adresa u EBX
    Emit(" MOV EAX,EBX");                          // Rezultat predstavlja adresu operanda
    break;                                         // 
  case '*':                                        // Operator pointera
    Novi();                                        // Pređi na novi znak
    Faktor();                                      // Rekurzivni poziv, rezultat u EAX, adresa u EBX
    Emit(" MOV EBX,EAX");                          // Sada se od vrijednosti dobije adresa
    Emit(" MOV EAX,[EBX]");                        // A zatim uzme vrijednost sa adrese
    break;                                         // 
  case '!':                                        // Logička negacija
    Novi();                                        // Pređi na novi znak
    Faktor();                                      // Rekurzivni poziv, desni operator, rezultat u EAX
    Emit(" CMP EAX,0");                            // Ako je EAX=0
    Emit(" SETE AL");                              // Tada prebaci 1 u AL
    Emit(" MOVZX EAX,AL");                         // I proširi AL na EAX
    break;                                         // 
  case '"':                                        // Dvostruki navodnici su string konstanta
    StringKonstanta();                             // Pozovi je za generisanje koda
    break;                                         // 
  case '(':                                        // Otvorena zagrada ima najveći prioritet
    Novi();                                        // Progutaj je
    IzrazDodjeljivanja();                          // I rekurzivno kreni opet od izraza dodjeljivanja
    MoraBiti(')');                                 // Na kraju mora biti uparena zatvorenom zagradpm
    break;                                         // 
  default:                                         // 
    if (isdigit(znak)) {                           // Ako je cifra
      sprintf(str1, " MOV EAX,%d",UzmiKonstantu());// Emituj dodjelu konstante u EAX
      Emit(str1);                                  // 
    } else if (isalpha(znak))                      // Ako je slovo
      Varijabla();                                 // Generiši varijablu ili poziv funkcije
    else                                           // 
      Greska("Greska u izrazu");                   // U ostalim slučajevima imamo grešku
  }                                                //
}                                                  // 
                                                   //
void Clan(void) {                                  // Clan obrađuje a*b, a/b, a%b rioritet 2
  unsigned char z;                                 // Ovdje pamtimo operator nakon prepoznavanja
  Faktor();                                        // Pozovi prvi operand, EAX će sadržati vrijednost
  while (strchr ("%/*", znak)) {                   // Dok je operator jedan od operatora 2. prioriteta
    z = znak;                                      // Zapamti operator
    Novi();                                        // Novi simbol
    Emit(" PUSH EAX");                             // Sačuvaj prvi operand
    Faktor();                                      // Pozovi drugi operand
    switch (z) {                                   // Zavisno od operatora
    case '*':                                      // Množenje:
      Emit(" POP EBX");                            // Vrati prvi faktor
      Emit(" IMUL EBX");                           // Ponoži drugi sa prvim, ignoriši viša 32 bita
      break;                                       //
    case '/':                                      // Dijeljenje i ostatak:
    case '%':                                      // 
      Emit(" MOV EBX,EAX");                        // Prebaci drugi faktor 
      Emit(" POP EAX");                            // Uzmi prvi faktor
      Emit(" CDQ");                                // Proširi dijeljenik na 64 bita
      Emit(" IDIV EBX");                           // Obavi dijeljenje, EAX je količnik, EDX ostatak
      if (z == '%')                                // Ako je tražen ostatak
        Emit(" MOV EAX,EDX");                      // Prebaci ga u EAX
      break;                                       //
    }                                              // 
  }                                                //
}                                                  // 
                                                   // Izraz obrađuje a+b, a-b, a&b, a|b i a^b, prioritet 3
void Izraz(void) {                                 // Nema parametara
  char z;                                          // Ovdje pamtimo operator nakon prepoznavanja
  Clan();                                          // Pozovi Clan da se izračuna prvi operand
  while (strchr ("^|&-+", znak)) {                 // Petlja se ponavlja dok je god znak jedan od navedenih
    z = znak;                                      // Zapamti dohvaćeni znak koji je operator
    Novi();                                        // Dohvati sljedeći znak
    Emit(" PUSH EAX");                             // Kako će nam trebati prvi operand, generiši čuvanje
    Clan();                                        // Dohvati sljedeći operand koji može biti složen
    switch (z) {                                   // Zavisno od operatora
    case '+':                                      // Sabiranje
      Emit(" POP EBX");                            // Vrati prvi sabirak
      Emit(" ADD EAX,EBX");                        // Kako je u EAX međurezultat, dobijemo kompletan zbir
      break;                                       //
    case '-':                                      // Oduzimanje
      Emit(" MOV EBX,EAX");                        // Nije komutativno, pa ćemo drugi operand staviti u EBX
      Emit(" POP EAX");                            // Vrati prvi operand
      Emit(" SUB EAX,EBX");                        // EAX sadrži rezultat oduzimanja
      break;                                       //
    case '&':                                      // Binarna konjunkcija
      Emit(" POP EBX");                            // Vrati prvi operand u EBX
      Emit(" AND EAX,EBX");                        // Kako je drugi operand već u EAX nakon AND imaćemo rezultat
      break;                                       //
    case '|':                                      // Binarna disjunkcija
      Emit(" POP EBX");                            // Vrati prvi operand u EBX
      Emit(" OR EAX,EBX");                         // Kako je drugi operand već u EAX nakon OR imaćemo rezultat
      break;                                       //
    case '^':                                      // Ekskluzivna disjunkcija
      Emit(" POP EBX");                            // Vrati prvi operand u EBX
      Emit(" XOR EAX,EBX");                        // Kako je drugi operand već u EAX nakon XOR imaćemo rezultat
      break;                                       //
    }                                              // Kraj petlje za obradu lijevo asocijativnih operatora
  }                                                //
}                                                  // 
                                                   //
void IzrazDodjeljivanja(void) {                    // Obrada operatora dodjele i relacionih operatora
  unsigned char z;                                 // Ovdje pamtimo operator nakon prepoznavanja
  int ChrIzraz;                                    // Tu pamti je li lijeva strana pointer na niz znakova
  DodjelaKarakteru = FALSE;                        // Normalno nije lijeva strana pointer na niz znakova
  Izraz();                                         // Lijeva strana, po izlazu EAX=vrednost, EBX=adresa
  switch (znak) {                                  // Pogledaj simbol na redu
  case 129:                                        // := Dodjeljivanje
    Novi();                                        // Idi na naredni simbol
    Emit(" PUSH EBX");                             // Generisi pamcenje adrese
    ChrIzraz = DodjelaKarakteru;                   // Zapamti da li je lijeva strana pointer na niz znakova
    IzrazDodjeljivanja();                          // Desna strana rekurzivno, izlaz EAX=vrednost, EBX=adresa
    Emit(" POP EBX");                              // Generiši vraćanje zapamćene adrese.
    if (ChrIzraz)                                  // Zavisno od vrste podatka
      Emit(" MOV [EBX],AL");                       // generiši upis znaka
    else                                           // 
      Emit(" MOV [EBX],EAX");                      // ili upis cijelog broja
    break;                                         // 
  case '=':                                        // Ako su u pitanju relacioni operatori jednako
  case '>':                                        // vece
  case '<':                                        // manje 
  case '#':                                        // razlicito
    Emit(" PUSH EAX");                             // Zapamti vrijednost lijeve strane
    z = znak;                                      // Zapamti i operator
    Novi();                                        // Novi simbol
    Izraz();                                       // Generisi desnu stranu
    Emit(" POP EBX");                              // Vrati vrijednost lijeve strane ali u EBX
    Emit(" CMP EBX,EAX");                          // Generiši poređenje lijeve i desne strane
    switch (z) {                                   // Zavisno o zapamćenom operatoru
    case '=':                                      // Jednako
      Emit(" SETE AL");                            // AL=1 ako je EBX=EAX , AL=0 ako ne
      break;                                       //
    case '>':                                      // Veće
      Emit(" SETG AL");                            // AL=1 ako je EBX>EAX , AL=0 ako ne
      break;                                       //
    case '<':                                      // Manje
      Emit(" SETL AL");                            // AL=1 ako je EBX<EAX , AL=0 ako ne
      break;                                       //
    case '#':                                      // Razlicito
      Emit(" SETNE AL");                           // AL=1 ako je EBX!=EAX , AL=0 ako ne
      break;                                       //
    }                                              // 
    Emit(" MOVZX EAX,AL");                         // Raširi AL u EAX da vrijednosti ostanu 0 ili 1
  }                                                //
}                                                  // 
                                                   //
void Uslov(void) {                                 // Implementacija naredbe "ako"
  char sinace[256], skraj[256], str[256];          // Varijable za tekst labela
  Novi();                                          // Naredni simbol
  IzrazDodjeljivanja();                            // Izraz uz uslov, vrati EAX kao rezultat 
  Emit(" CMP EAX,0");                              // Generiši poređenje s nulom
  NovaLabela(sinace);                              // Pripremi dvije labele
  NovaLabela(skraj);                               // 
  sprintf(str, " JZ %s", sinace);                  // Generiši uslovni skok na labelu za "inace"
  Emit(str);                                       // Emituj naredbu
  MoraBiti('{');                                   // Očekuj vitičastu zagradu
  Blok();                                          // Pozovi blok
  if (znak == 133) {                               // Ako ima ključne riječi "inace"
    sprintf(str, " JMP %s", skraj);                // Pripremi naredbu za preskok ove sekcije
    Emit(str);                                     // Emituj je
    EmitL(sinace);                                 // Emituj labelu za "inace"
    Novi();                                        // Novi simbol
    MoraBiti('{');                                 // Očekuj novu vitičastu zagradu
    Blok();                                        // Pozovi Blok
    EmitL(skraj);                                  // Emituj labelu za kraj uslova
  } else                                           // Ako nije bilo naredbe "inace"
    EmitL(sinace);                                 // tada samo emituj labelu
}                                                  // 
                                                   //
void Petlja(void) {                                // Implementacija naredbe "dok"
  char skraj [256], suslov[256], str[256];         // Varijable za tekst labela
  Novi();                                          // Naredni simbol nakon tokena
  NovaLabela(suslov);                              // Generiši labelu prije testa uslova
  EmitL(suslov);                                   // I odmah je emituj
  IzrazDodjeljivanja();                            // Pozovi kontrolni izraz
  Emit(" CMP EAX,0");                              // Nakon izraza testiraj EAX, rezultat
  NovaLabela(skraj);                               // Generiši labelu za iza bloka, ali je ne emituj
  sprintf(str, " JZ %s", skraj);                   // Nego pripremi skok gdje se ide ako je EAX=0
  Emit(str);                                       // I emituj taj skok
  MoraBiti('{');                                   // Sada slijedi vitičasta zagrada prije bloka
  Blok();                                          // Pozovi Blok
  sprintf(str, " JMP %s", suslov);                 // Na kraju bloka generiši skok na početak testa
  Emit(str);                                       // Sada se emituje skok na test
  EmitL(skraj);                                    // A zatim labela na koju se skoči nakon testa
}                                                  // 
                                                   //
void Asembler(void) {                              // Implementacija naredbe "asembler"
  do {                                             // Ovdje moramo odustati od Novi funkcije
    fgets(linija, 256, ulaz);                      // Nego se kupi linija po linija teksta
    if (linija[0] != '\\')                         // do linije koja počinje naopakom kosom crtom
      Emit(linija);                                // i bez obrade šalje na izlaz
    if (feof(ulaz))                                // Ipak, na kraju ulazne datoteke
      strcpy(linija, "\\");                        // simulira se da je bila naopaka kosa crta
  } while (linija[0] != '\\');                     // Petlja se ponavlja do naopake kose crte
  pozicija = 0;                                    // Sada su ponovo normalne linije, resetuj poziciju
  znak = linija[pozicija];                         // Uzmi znak
  dlin = strlen(linija);                           // Računaj dužinu
  MoraBiti('\\');                                  // I sinhronizuj nakon naopake kose crte.
}                                                  // 
                                                   //
void Blok(void) {                                  // Ova funkcija je pumpa sintaksnog analizatora
  while (znak != '}') {                            // Blok traje do zatvorene vitičaste zagrade
    switch(znak) {                                 // Provjeri znak
    case 131:                                      // Ključna riječ "dok"
      Petlja();                                    // Aktiviraj petlju
      break;                                       //
    case 130:                                      // Ključna riječ "ako"
      Uslov();                                     // Aktiviraj uslov
      break;                                       //
    case 132:                                      // Ključna riječ "asembler"
      Asembler();                                  // Pozovi asembler
      break;                                       //
    default:                                       //
      IzrazDodjeljivanja();                        // U svim ostalim slučajevima, izraz dodjeljivanja
      MoraBiti(';');                               // On mora imati tačka-zarez na kraju
    }                                              // 
  }                                                //
  MoraBiti('}');                                   // Provjeri finalnu vitičastu zagradu
}                                                  // 
                                                   //
void DefFunkcija(void) {                           // Definicija funkcije je najkompleksniji potprogram
  char ImeF[256], ImeVar[256], str1[256];          // Pomoćne varijable
  int paradresa = 0, lokadresa;                    // Brojači pozicije parametara i lokalnih varijabli
  UzmiIme(ImeF);                                   // Dohvati ime funkcije
  strcpy(aktfun, ImeF);                            // Njeno ime se lijepi uz imena lokalnih varijabli
  MoraBiti('(');                                   // Očekuj otvorenu zagradu
  while (znak != ')') {                            // Sve do zatvorene zagrade
    UzmiIme(ImeVar);                               // Dohvati ime formalnog parametra
    paradresa += 4;                                // Uvećaj poziciju parametra
    strcat(ImeVar, aktfun);                        // Nadodaj ime funkcije na ime parametra
    if (Unesen(ImeVar))                            // Ako je već u tabeli simbola
      Greska("Vec definisano");                    // prijavi grešku
    DodajSimbol(ImeVar, LOK);                      // Inače dodaj simbol da je lokalna varijabla
    sprintf(str1, "%s EQU %d",ImeVar, paradresa+4);// A relativni pomak od EBP će biti paradresa
    Emit(str1);                                    // Emituj direktivu koja dodjeljuje labelu
    if (znak != ')')                               // Sada ako ne slijedi zatvorena zagrada
      MoraBiti(',');                               // Onda mora biti zarez
  }                                                //
  MoraBiti(')');                                   // Kraj parametara
  if (Unesen(ImeF))                                // Provjeri da li je funkcija već definisana
    Greska("Vec definisano");                      // Ako jeste, greška
  DodajSimbol(ImeF, FUN);                          // Funkciju ubacujemo u tabelu simbola
  switch (znak) {                                  // Ako je znak iza definicije funkcije 
  case '{':                                        // vitičasta zagrada, to je tijelo funkcije
    MoraBiti('{');                                 // Vitičasta zagrada
    lokadresa = 0;                                 // Brojač lokalnih varijabli, svaka uzima 4 bajta
    if (znak == 134) {                             // Ako je ključna riječ 'varijable'
      Novi();                                      // Novi simbol
      do {                                         // 
        UzmiIme(ImeVar);                           // Dohvati ime varijable
        lokadresa += 4;                            // Uvećaj brojač lokalnih adresa
        strcat(ImeVar, aktfun);                    // Nadodaj ime funkcije na ime varijable
        if (Unesen(ImeVar))                        // Ako je takvo ime već uneseno
          Greska("Vec definisan");                 // generiši grešku
        DodajSimbol(ImeVar, LOK);                  // Ubaci simbol kao lokalnu varijablu
        sprintf(str1, "%s EQU %d",                 // Pripremi simbol sa imenom varijable čije će
                ImeVar, -lokadresa);               // će vrijednost biti negativni pomak od EBP
        Emit(str1);                                // Emituj taj simbol
        if (znak != ';')                           // Varijable se završavaju tačkazarezom
          MoraBiti(',');                           // A ako ne, razdvojene su zarezima
      } while (znak != ';');                       // Sve dok ne dođemo do tačkazareza.
      MoraBiti(';');                               // I njega pročitamo
    }                                              // 
    sprintf(str1, "GLOBAL %s",ImeF);               // Ime funkcije deklariši kao globalnu labelu
    Emit(str1);                                    // Emituj GLOBAL direktuvu
    sprintf(str1, "%s:", ImeF);                    // A zatim i samu labelu sa ulazom u funkciju
    Emit(str1);                                    //
    Emit(" PUSH EBP");                             // Generiši standardni početak 32 bitnih funkcija
    Emit(" MOV EBP,ESP");                          // Pamćenje linka
    sprintf(str1, " SUB ESP,%d",lokadresa);        // Alokacija lokalnih varijabli
    Emit(str1);                                    // Emituj i treću instrukciju ulaza
    Blok();                                        // Tijelo funkcije
    Emit(" MOV ESP,EBP");                          // Na kraju funkcije se vrate ESP
    Emit(" POP EBP");                              // i EBP
    sprintf(str1, " RET %d", paradresa);           // Pripremi RET instrukciju sa čišćenjem parametara
    Emit(str1);                                    // Emituj je
    break;                                         // Kraj varijante sa tijelom funkcije
  case ';':                                        // tačkazarez iza definicije je eksterma funkcija
    sprintf(str1, "extern %s", ImeF);              // tada samo deklarišemo extern direktivu
    Emit(str1);                                    // emitujemo je
    MoraBiti(';');                                 // i očekujemo tačkazarez
    break;                                         // Kraj varijante sa eksternom deklaracijom
  default:                                         // Nije ni vitičasta zagrada ni tačkazarez
    Greska("Sintaksa funkcije");                   // To je greška
    break;                                         // 
  }                                                //
}                                                  // 
                                                   //
void Prevedi(void) {                               // Prevođenje cijelog programa
  char ImeVar[256];                                // Pomoćne varijable
  char str1[256];                                  // 
  Emit( "EXTERN ExitProcess");                     // Imaćemo makar ExitProcess API funkciju
  Emit("section .data");                           // Prelazimo u deklaraciju varijabli
  pozicija = -1;                                   // Da se forsira čitanje linije sa ulaza
  Novi();                                          // Dohvati prvi simbol
  if (znak == 134) {                               // Je li to ključna riječ "varijable"
    Novi();                                        // Ako jeste uzmi naredni simbol
    do {                                           // I sve dok ima varijabli
      if (isalpha(znak)) {                         // Ako simbol počinje slovom
        UzmiIme(ImeVar);                           // Dohvati ime varijable
        if (Unesen(ImeVar))                        // Ako je već u tabeli simbola
          Greska("Vec definisan");                 // Prijavi grešku
        DodajSimbol(ImeVar, GLOB);                 // Inače ubaci simbol
        sprintf(str1, "%s DD 0", ImeVar);          // Inicijaliziraj globalnu varijablu na 0
        Emit(str1);                                // Emituj DD naredbu
        if (znak != ';')                           // Nastavi do tačka zareza, 
          MoraBiti(',');                           // ali se varijable razdvajaju zarezima
      }                                            // 
    } while (znak != ';');                         // Kada smo došli do kraja varijabli
    MoraBiti(';');                                 // Progutaj tačkazarez
  }                                                //
  Emit("section .text");                           // Sada ide kodna sekcija
  while (znak == 135) {                            // Dokle god je ključna riječ "funkcija"
    Novi();                                        // Novi simbol
    DefFunkcija();                                 // Pozovi definiciju funkcije
  }                                                //
  strcpy(aktfun, "_");                             // Glavni program,obriši varijablu aktivne funkcije
  MoraBiti('{');                                   // Obavezna vitičasta zagrada
  if (znak == '}')                                 // Ako je iza nje zatvorena vitičasta
    return;                                        // to je biblioteka i nema startnog koda
  Emit(" global start");                           // Inače dodajemo ulaznu tačku
  Emit("start:");                                  // Emitujemo labelu
  Blok();                                          // Blok sa naredbama glavnog programa
  Emit(" PUSH 0");                                 // Proslijedi parametar 0 za ExitProcess
  Emit(" CALL ExitProcess");                       // API funkcija za završetak programa
}                                                  // 
                                                   //
void Glavni(void) {                                // Omotač za funkciju Prevedi
  trenlabela = 0;                                  // Inicijaliziraj brojač labela
  brojaclinija = 0;                                // Inicijaliziraj brojač linija
  *linija = '\0';                                  // Isprazni liniju
  Prevedi();                                       // Pozovi Prevedi
  Emit("END_:");                                   // Labela kraja datoteke, trebaće nekad
  fclose(ulaz);                                    // Zatvori ulaznu datoteku
  fclose(izlaz);                                   // Zatvori izlaznu datoteku 
  printf("Prevedeno! \n");                         // Pozdravna poruka
}                                                  // 
                                                   //
int main(int argc, char *argv[]) {                 // Glavni program očita parametre
  if (argc == 3) {                                 // Ima li dva parametra
    if(!(ulaz = fopen(argv[1], "rb"))) {           // Prvi parametar, ulazna datoteka, probaj otvoriti
      fprintf(stderr, "Necitljiva %s \n", argv[1]);// Neuspjeh otvaranja ulazne datoteke
      exit(0);                                     // Prekid
    }                                              // 
    if(!(izlaz = fopen(argv[2], "wb"))) {          // 2. parametar, izlazna datoteka, probaj otvoriti
      fprintf(stderr, "Neupisiva %s \n", argv[2]); // Neuspjeh otvaranja izlazne datoteke
      exit(0);                                     // 
    }                                              // 
  } else {                                         // 
    fprintf(stderr, "KOMP ime.fil ime.asm \n");    // Nisu proslijeđena dva parametra, greška
    exit(0);                                       //
  }                                                //
  Glavni();                                        // Pokreni kompajliranje
  return(0);                                       // Vrati se u operativni sistem
}                                                  // 
                                                   //