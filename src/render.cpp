#include "inc.h"

TTF_Font * ssubfont;
TTF_Font * subfont;
TTF_Font * mainfont;
TTF_Font * bigopfont;
TTF_Font * auxfont;

bool GetErrors(Symat * symat,char * config){
	initSymat(symat,config);

	if(SDL_Init(SDL_INIT_VIDEO) < 0){
		printf("ErrorSDL: %s\n",SDL_GetError());
		return true;
	}
	symat->window = SDL_CreateWindow("Symat",1000,
			40, 900,300,SDL_WINDOW_SHOWN);
			// SCREEN_WIDTH,
			// SCREEN_HEIGHT,
			// SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	symat->renderer = SDL_CreateRenderer(symat->window,-1,SDL_RENDERER_ACCELERATED);
	symat->altrenderer = SDL_CreateRenderer(symat->window,-1,SDL_RENDERER_ACCELERATED);
	if(symat->window == NULL || symat->renderer == NULL || TTF_Init() == -1){
		printf("ErrorSDL: %s\n",SDL_GetError());
		printf("ErrorSDL: %s\n",SDL_GetError());
		return true;
	} else {
		//TODO: Hacer que elija la versión otf o ttf si no encuentra su versión ttf u otf.
		auxfont = TTF_OpenFont("auxfont.ttf",AUXFONTSIZE);
		ssubfont = TTF_OpenFont("font.otf",SSUBFONTSIZE);
		subfont = TTF_OpenFont("font.otf",SUBFONTSIZE);
		mainfont = TTF_OpenFont("font.otf",FONTSIZE);
		bigopfont = TTF_OpenFont("font.otf",BIGOPFONTSIZE);
		if(mainfont == NULL || bigopfont == NULL|| subfont == NULL|| ssubfont == NULL|| auxfont == NULL){
			auxfont = TTF_OpenFont("auxfont.ttf",AUXFONTSIZE);
			ssubfont = TTF_OpenFont("font.ttf",SSUBFONTSIZE);
			subfont = TTF_OpenFont("font.ttf",SUBFONTSIZE);
			mainfont = TTF_OpenFont("font.ttf",FONTSIZE);
			bigopfont = TTF_OpenFont("font.ttf",BIGOPFONTSIZE);
			if(mainfont == NULL || bigopfont == NULL|| subfont == NULL|| ssubfont == NULL|| auxfont == NULL){
				printf("error, no hay fuente\n");
				exit(1);
			}
		}
		symat->screenSurface = SDL_GetWindowSurface(symat->window);
		SDL_GetWindowSize(symat->window, symat->screenx, symat->screeny);
		SDL_FillRect(symat->screenSurface,NULL,
			SDL_MapRGB(symat->screenSurface->format,255,255,255));
		SDL_UpdateWindowSurface(symat->window);
	}
	return false;
}

// Portear para que forme un constructor
void initSymat(Symat * symat, char * config){
	symat->quit = false;
	symat->screenx  = (int*)malloc(sizeof(int));
	symat->screeny  = (int*)malloc(sizeof(int));
	symat->movex    = (int*)malloc(sizeof(int));
	symat->prevhmax = (int*)malloc(sizeof(int));
	*symat->movex = 0;
	*symat->prevhmax = 0;
	symat->updatewin = true;
	symat->buffer = std::make_unique<Buffer[]>(16);
	symat->conf = std::make_unique<Conf>();
	symat->conf->bufferpos = 0;
	symat->conf->posx = 0;
	symat->conf->posy = 0;
	symat->numtokens = getNumTokens(config);
	symat->tokens = getTokens(config,symat->numtokens);

}

//TODO: HACER MAS DE 2 FRACCIONES ANIDADAS
//FIXME: OPTIMIZAR haciendo que dibuje los buffers primero y los guarde, luego haga el calculo
//		de dimension para las fracciones y copie las texturas en la panta.a.
void drawBuffer(Symat& symat, Buffer& buffer, float factor, int spanx, int spany,
                int* movex, int* prevhmax, bool firsttime, SDL_Color colorfg, SDL_Color colorbg, bool draw) {

    float dimfactor = (4 * factor) / 5;
    int chary = 0;

    if (!buffer.tokens.empty()) {
        SDL_Surface* chartest = nullptr;

        const auto& lastToken = buffer.tokens.back();
        if ((lastToken.flags & ISBIGOP) == ISBIGOP) {
            chartest = TTF_RenderUTF8_Shaded(bigopfont, lastToken.token.c_str(), colorfg, colorbg);
        } else {
            chartest = TTF_RenderUTF8_Shaded(mainfont, lastToken.token.c_str(), colorfg, colorbg);
        }

        if (chartest != nullptr) {
            chary = static_cast<int>(factor * chartest->h);
            SDL_FreeSurface(chartest);
        }
    }

    drawLine(symat, buffer, buffer.tokens, buffer.tokens.size(), factor, spanx, spany, buffer.size.y, movex, prevhmax, colorfg, colorbg, draw);

    int supx = 0;
    int subx = 0;
    int supy = 0;
    int suby = 0;
    int fracsupx = 0;
    int fracsubx = 0;
    int fracsupy = 0;
    int fracsuby = 0;
    int inchmax = 0;
    int fracmaxy = 0;

    if (buffer.suptext) {
        drawBuffer(symat, *buffer.suptext, dimfactor,
                   spanx + buffer.size.x - factor * 3,
                   spany - chary / 3, &supx, &supy, false, colorfg, colorbg, draw);
    }

    if (buffer.subtext) {
        drawBuffer(symat, *buffer.subtext, dimfactor,
                   spanx + buffer.size.x - factor * 3,
                   spany + (2 * chary) / 3, &subx, &suby, false, colorfg, colorbg, draw);
    }

    buffer.size.x += std::max(supx, subx);
    buffer.size.y = std::max(buffer.size.y, chary + supy + suby);

    *movex += std::max(supx, subx);

    if (buffer.fracsuptext) {
        drawBuffer(symat, *buffer.fracsuptext, factor,
                   spanx + buffer.size.x - factor * 3,
                   spany - FONTSIZE / 2, &fracsupx, &fracsupy, false, colorfg, colorbg, false); //
        fracmaxy += buffer.fracsuptext->size.y;
    	if (buffer.fracsubtext) {
        	drawBuffer(symat, *buffer.fracsubtext, factor,
        	           spanx + buffer.size.x - factor * 3,
        	           spany + FONTSIZE / 2, &fracsubx, &fracsuby, false, colorfg, colorbg, false);
        	fracmaxy += buffer.fracsubtext->size.y;
		}
    }

    buffer.size.y = std::max(buffer.size.y, fracmaxy);

    if (buffer.fracsuptext) {
        if (buffer.fracsuptext->size.x > buffer.fracsubtext->size.x) {
            drawBuffer(symat, *buffer.fracsuptext, factor,
                       10 + spanx + buffer.size.x - factor * 3,
                       spany - buffer.fracsuptext->size.y / 2, &fracsupx, nullptr, false, colorfg, colorbg, draw);
			if(draw){
            	SDL_Rect rect = {10 + spanx + buffer.size.x - factor * 3, spany + buffer.fracsuptext->size.y / 2,
                             buffer.fracsuptext->size.x, 3};
            	SDL_SetRenderDrawColor(symat.renderer, 0, 0, 0, 255);
            	SDL_RenderFillRect(symat.renderer, &rect);
			}
            if (buffer.fracsubtext) {
                drawBuffer(symat, *buffer.fracsubtext, factor,
                           10 + spanx + buffer.size.x - factor * 3,
                           spany + buffer.fracsubtext->size.y / 2, &fracsubx, nullptr, false, colorfg, colorbg, draw);
            }
        } else if (buffer.fracsuptext->size.x <= buffer.fracsubtext->size.x) {
            drawBuffer(symat, *buffer.fracsuptext, factor,
					10 + spanx + buffer.size.x - factor * 3 + (buffer.fracsubtext->size.x - buffer.fracsuptext->size.x) / 2,
				   	spany - buffer.fracsuptext->size.y / 2 - 3, &fracsupx, nullptr, false, colorfg, colorbg, draw); // Parte superior
			if(draw){
            	SDL_Rect rect = {10 + spanx + buffer.size.x - factor * 3,
					spany + buffer.fracsuptext->size.y / 2, buffer.fracsubtext->size.x, 3}; // barra de fraccion
            	SDL_SetRenderDrawColor(symat.renderer, 0, 0, 0, 255);
            	SDL_RenderFillRect(symat.renderer, &rect); // Dibuja la barra
			}
            drawBuffer(symat, *buffer.fracsubtext, factor, 10 + spanx + buffer.size.x - factor * 3,
					spany + buffer.fracsubtext->size.y / 2 + 3, &fracsubx, nullptr, false, colorfg, colorbg, draw); // Dibuja la parte inferior
        }
    }

    buffer.size.x += std::max(fracsupx, fracsubx);

    if (buffer.fracsuptext) {
        *movex += buffer.fracsubtext->size.x;
    }
    if (buffer.fracsubtext) {
        *movex += -buffer.fracsubtext->size.x + std::max(buffer.fracsubtext->size.x, buffer.fracsuptext->size.x) + 20;
    }

    if (buffer.next) {
        drawBuffer(symat, *buffer.next, factor, spanx + *movex, spany, movex, prevhmax, false, colorfg, colorbg, draw);
    }
}


void drawLine(Symat& symat, Buffer& buffer, std::vector<Token> tokens, int maxtokens,float factor,int spanx,int spany,int medy,int * movex,int *prevhmax,SDL_Color colorfg,SDL_Color colorbg,bool draw){
    SDL_Texture * dummyTexture;
    SDL_Rect * rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
    int hmax = 0;
    if(prevhmax != NULL)
        hmax = *prevhmax;
    rect->y = spany; 
    rect->x = spanx;
    for(int i = 0; i < maxtokens; i++){                   
        if((tokens[i].flags & ISBIGOP) == ISBIGOP){       
            SDL_Surface * dummySurface = TTF_RenderUTF8_Shaded(bigopfont,
					tokens[i].token.c_str(),colorfg,colorbg);                                                                                                          
            rect->w = dummySurface->w * factor;           
            rect->y = spany;                              
            rect->h = dummySurface->h * factor;           
            if(rect->h > hmax){
                hmax = rect->h;                           
            }   
            dummyTexture = SDL_CreateTextureFromSurface(symat.renderer,dummySurface);                                                                                                                                                     
			if(draw)
            	SDL_RenderCopy(symat.renderer,dummyTexture,0,rect);
            if(dummySurface != NULL){                     
            	rect->x = rect->x + dummySurface->w * factor; 
			}
            SDL_DestroyTexture(dummyTexture);             
            SDL_FreeSurface(dummySurface);
        } else {                                          
            SDL_Surface * dummySurface = TTF_RenderUTF8_Shaded(mainfont,
					tokens[i].token.c_str(),colorfg,colorbg);                                                                                                                         
            if(dummySurface == NULL){                     
                rect->w = 0;                              
                rect->y = spany;                          
                rect->h = 0;                              
            } else {
                rect->w = dummySurface->w * factor;       
                rect->h = dummySurface->h * factor;       
            	if(rect->h > hmax){
            	    hmax = rect->h;                           
					// Volver a iniciar.
            	}   
				rect->y = spany + (hmax-rect->h)/2; //FIXME: Calcula la media posterior al mas grande, pero no vuelve a analizar el resto de los primeros elementos.
            }
            dummyTexture = SDL_CreateTextureFromSurface(symat.renderer,dummySurface);                                                                                                                                                     
			if(draw)
            	SDL_RenderCopy(symat.renderer,dummyTexture,0,rect);
            if(dummySurface != NULL){                     
                rect->x = rect->x + dummySurface->w * factor;
            }                                             
            SDL_DestroyTexture(dummyTexture);             
            SDL_FreeSurface(dummySurface);                
        }
    }
    buffer.size.x = rect->x-spanx;
    if(prevhmax != NULL){
        *prevhmax = hmax;
	}
    if(movex != NULL)
        *movex = rect->x-spanx;
    buffer.size.y == std::max(buffer.size.y,hmax);
    free(rect);
}


void drawParse(Symat& symat,Buffer& buffer,SDL_Color colorfg, SDL_Color colorbg){
	SDL_Surface * texttoprint = TTF_RenderUTF8_Shaded(auxfont, buffer.parsetext.c_str(), colorfg,colorbg);
	if(texttoprint != NULL){
		SDL_Rect rect = {0,0,texttoprint->w,texttoprint->h};
		SDL_Texture * dummyTexture = SDL_CreateTextureFromSurface(symat.renderer,texttoprint);
		SDL_RenderCopy(symat.renderer,dummyTexture,0,&rect);
		SDL_FreeSurface(texttoprint);
		SDL_DestroyTexture(dummyTexture);
	}
}
