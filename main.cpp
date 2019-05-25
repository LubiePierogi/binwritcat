#include <unistd.h>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <cstdio>

struct Fajne {
  using intt = intptr_t;
  static constexpr intt MAX_ESC_SIZE = 4;
  static constexpr intt BUF_SIZE = MAX_ESC_SIZE + 2;
  enum class Meow : int16_t {
    NORMALNIE = '\0',
    BACKSLASH = '\\',
    HEX = 'x',
    OCT = '0',
    PRZEKLEJONE = (1 << 8) + 0,
  };
  char buf[BUF_SIZE];
  intt gdzie = 0;
  bool end = false;
  bool slij = false;
  Meow meow = Meow::NORMALNIE;
  char &x(intptr_t n) {return buf[n];}
  char &xx() {return x(gdzie);}
  void r() {
    // gdzie = HEH;
    // escape = '\0';
    gdzie = 0;
    memset(buf, '\0', sizeof(buf));
    meow = Meow::NORMALNIE;
  }
  void set_len(intt n) {
    gdzie = n;
    memset(&buf[n], '\0', sizeof(buf) - n);
  }
  char char_to_print;

  void set_char(char c) {
    char_to_print = c;
    r();
    slij = true;
  }

  void przeklejando(char c, char c2) {
    char_to_print = c;
    x(0) = c2;
    set_len(1);
    gdzie = 0;
    slij = true;
    meow = Meow::PRZEKLEJONE;
  }

  /*
  intptr_t gdzie;
  char char_to_print;
  char escape;
  Fajne() : gdzie(0), char_to_print('\0'), escape('\0') {
    memset(buf, '\0', sizeof(buf));
  }
  */
  int mrau();
  /*
  void mrau_escape();
  void mrau_nei_escape();
  void mrau_hex();
  void mrau_oct();
*/

  bool wysylac() {return slij;}
  void wyslij() {
    if (write(STDOUT_FILENO, &char_to_print, 1) != 1) {
      // Chyba nie można już pisać xd
      end = true;
    }
  }
  void daj_znak();
};


void Fajne::daj_znak() {
  while (true) {
    switch (meow) {
      case Meow::NORMALNIE:
        assert(gdzie == 0);
        if (read(STDIN_FILENO, &xx(), 1) != 1) {
          end = true;
          return;
        }
      case Meow::PRZEKLEJONE:  // to jest celowe
        if (xx() == '\\') {
          meow = Meow::BACKSLASH;
          ++gdzie;
          break;
        } else if (xx() == '\n') {
          r();
          break;
        }
        set_char(xx());
        return;
      case Meow::BACKSLASH:
        assert(gdzie == 1);
        assert(x(0) == '\\');
        if (read(STDIN_FILENO, &xx(), 1) != 1) {
          set_char('\\');
          end = true;
          return;
        }
        switch (xx()) {
          case 'n':
            set_char('\n');
            return;
          case 't':
            set_char('\t');
            return;
          case 'v':
            set_char('\v');
            return;
          case 'r':
            set_char('\r');
            return;
          case 'f':
            set_char('\f');
            return;
          case '\\':
            set_char('\\');
            return;
          case 'x':
            meow = Meow::HEX;
            ++gdzie;
            break;
          case '0':
            meow = Meow::OCT;
            ++gdzie;
            break;
          case '\n':
            r();
            break;
          default:
            przeklejando('\\', xx());
            return;
        }
        break;
      case Meow::HEX:
        assert(gdzie >= 2);
        assert(x(0) == '\\');
        assert(x(1) == 'x');
        if (read(STDIN_FILENO, &xx(), 1) != 1) {
          if (gdzie == 2) {
            // No ok, wyślijmy nic.
            r();
          } else {
            int znak = '\0';
            sscanf(&buf[2], "%x", &znak);
            set_char(znak);
          }
          end = true;
          return;
        }
        if (!isxdigit(xx())) {
          // No, trafiliśmy na coś, co nie jest naszą cyfrą, trzeba zrobić
          // przeklejando xd
          char koncoweczka = xx();
          xx() = '\0';
          int znak;
          if (gdzie > 2) {
            sscanf(&buf[2], "%x", &znak);
          } else {
            znak = '\0';
          }
          przeklejando(znak, koncoweczka);
          return;
        } else if (gdzie >= 3) {
          // ostatni znak xd
          int znak;
          sscanf(&buf[2], "%x", &znak);
          set_char(znak);
          return;
        } else {
          ++gdzie;
          break;
        }
      case Meow::OCT:
        assert(gdzie >= 2);
        assert(x(0) == '\\');
        assert(x(1) == '0');
        if (read(STDIN_FILENO, &xx(), 1) != 1) {
          // no, chyba koniec
          if (gdzie == 2) {
            // No ok, wyślijmy nic.
            r();
          } else {
            int znak = '\0';
            sscanf(&buf[1], "%o", &znak);
            set_char(znak);
          }
          end = true;
          return;
        }
        if (!isdigit(xx()) || xx() == '8' || xx() == '9') {
          // No, trafiliśmy na coś, co nie jest naszą cyfrą, trzeba zrobić
          // przeklejando xd
          char koncoweczka = xx();
          xx() = '\0';
          int znak;
          sscanf(&buf[1], "%o", &znak);
          przeklejando(znak, koncoweczka);
          return;
        } else if (gdzie >= 4) {
          // ostatni znak xd
          int znak;
          sscanf(&buf[1], "%o", &znak);
          set_char(znak);
          return;
        } else {
          ++gdzie;
          break;
        }
    }
  }
}

int Fajne::mrau() {
  do {
    daj_znak();
    if (wysylac()) {
      wyslij();
      slij = false;
    }
  } while (!end);

  return 0;
}


Fajne fajne;

int main(int argc, char **argv) {
  return fajne.mrau();
}
