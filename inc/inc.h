#ifndef LIBS
#define LIBS

#include <iostream>
#include <regex>
#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <unistr.h>
#include <fstream>
#include <locale>

#endif

#ifndef CONSTS
#define CONSTS

#define ISBIGOP			0b00000001
#define ISPARENTHESIS	0b00000010
#define ISFRAC			0b00000100
#define CLOSEPAR		0b00001000
#define MAKESUP			0b00010000
#define MAKESUB			0b00100000

#define AUXFONTSIZE 32
#define FONTSIZE 64
#define SUBFONTSIZE 40
#define SSUBFONTSIZE 51
#define BIGOPFONTSIZE 96

#endif

#ifndef ASTRUCTS
#define ASTRUCTS

typedef struct {
	int x;
	int y;
} iint;

typedef struct {
	int bufferpos;
	int posx;
	int posy;
} Conf;

typedef struct {
	uint8_t flags;
	std::string token;
	std::string chara;
} Token;

class Buffer {
public:
	int dim = 0;
    iint pos = {0, 0};
    iint size = {0, 0};
    iint span = {0, 0};
    std::string parsetext;
    std::vector<Token> tokens;
    std::unique_ptr<Buffer> subtext;
    std::unique_ptr<Buffer> suptext;
    std::unique_ptr<Buffer> fracsubtext;
    std::unique_ptr<Buffer> fracsuptext;
    std::unique_ptr<Buffer> next;

    // Constructor por defecto
    Buffer() = default;

    // Constructor con inicialización opcional de parsetext
    explicit Buffer(const std::string& text)
        : parsetext(text) {}

    // Constructor de copia
    Buffer(const Buffer& other)
        : pos(other.pos),
          size(other.size),
          span(other.span),
          parsetext(other.parsetext),
          tokens(other.tokens),
          subtext(other.subtext ? std::make_unique<Buffer>(*other.subtext) : nullptr),
          suptext(other.suptext ? std::make_unique<Buffer>(*other.suptext) : nullptr),
          fracsubtext(other.fracsubtext ? std::make_unique<Buffer>(*other.fracsubtext) : nullptr),
          fracsuptext(other.fracsuptext ? std::make_unique<Buffer>(*other.fracsuptext) : nullptr),
          next(other.next ? std::make_unique<Buffer>(*other.next) : nullptr) {}

    // Constructor de movimiento
    Buffer(Buffer&& other) noexcept = default;

    // Operadores de asignación
    Buffer& operator=(const Buffer& other) {
        if (this == &other) return *this;
        pos = other.pos;
        size = other.size;
        span = other.span;
        parsetext = other.parsetext;
        tokens = other.tokens;
        subtext = other.subtext ? std::make_unique<Buffer>(*other.subtext) : nullptr;
        subtext = other.subtext ? std::make_unique<Buffer>(*other.subtext) : nullptr;
        fracsuptext = other.fracsuptext ? std::make_unique<Buffer>(*other.fracsuptext) : nullptr;
        fracsuptext = other.fracsuptext ? std::make_unique<Buffer>(*other.fracsuptext) : nullptr;
        next = other.next ? std::make_unique<Buffer>(*other.next) : nullptr;
        return *this;
    }

    Buffer& operator=(Buffer&& other) noexcept = default;
};

typedef struct {
	bool quit;
	bool updatewin;
	int numtokens;

	int * screenx;
	int * screeny;
	int * movex;
	int * prevhmax;

	SDL_Window * window;
	SDL_Renderer * renderer;
	SDL_Renderer * altrenderer;
	SDL_Surface * screenSurface;
	//Buffer * bufferhelp;
	std::unique_ptr<Buffer[]> buffer;
	std::vector<Token> tokens;
	std::unique_ptr<Conf> conf;
} Symat;

#endif

#ifndef FUNCS
#define FUNCS

bool GetErrors(Symat * symat,char * config);
void initSymat(Symat * symat, char * config);
void updateBuffer(Token *dest,int destlen, Buffer *origin, Token * tokens);
float getfactor(char * num);
Buffer initBuffer();
Buffer * setBuffer();
void triggerEasterEgg(Symat * symat,int n);
void UpdateSymat(Symat * symat);

void readTokens(Symat& symat, Buffer& dest);
std::vector<Token> getTokens(const char* config, int numtokens);
std::string remove_last_utf8_char(const std::string& str);
int getNumTokens(const std::string & config);
void remove_last_utf8_char(std::string& str);
void drawLine(Symat& symat, Buffer& buffer, std::vector<Token> tokens, int maxtokens,float factor,int spanx,int spany,int medy,int * movex,int *prevhmax,SDL_Color colorfg,SDL_Color colorbg,bool draw);
void drawBuffer(Symat& symat, Buffer& buffer, float factor, int spanx, int spany, int* movex, int* prevhmax, bool firsttime, SDL_Color colorfg, SDL_Color colorbg,bool draw);



//void drawLine(Symat& symat, Buffer& buffer, std::vector<Token> tokens, float factor, int spanx, int spany, int medy, int* movex, int* prevhmax, SDL_Color colorfg, SDL_Color colorbg);
void drawParse(Symat& symat,Buffer& buffer,SDL_Color colorfg, SDL_Color colorbg);

#endif
