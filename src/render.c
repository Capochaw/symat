#include "inc.h"

TTF_Font * mainfont;
TTF_Font * bigopfont;

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
	if(symat->window == NULL || symat->renderer == NULL || TTF_Init() == -1){
		printf("ErrorSDL: %s\n",SDL_GetError());
		printf("ErrorSDL: %s\n",SDL_GetError());
		return true;
	} else {
		mainfont = TTF_OpenFont("font.otf",FONTSIZE);
		bigopfont = TTF_OpenFont("font.otf",BIGOPFONTSIZE);
		if(mainfont == NULL || bigopfont == NULL){
			mainfont = TTF_OpenFont("font.ttf",FONTSIZE);
			bigopfont = TTF_OpenFont("font.ttf",BIGOPFONTSIZE);
			if(mainfont == NULL || bigopfont == NULL){
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

void initSymat(Symat * symat, char * config){
	symat->quit = false;
	symat->screenx = malloc(sizeof(int));
	symat->screeny = malloc(sizeof(int));
	symat->movex = malloc(sizeof(int));
	symat->prevhmax = malloc(sizeof(int));
	*symat->movex = 0;
	*symat->prevhmax = 0;
	symat->updatewin = true;
	symat->bufferhelp = malloc(sizeof(Buffer));
	symat->bufferhelp = setBuffer();
	symat->buffer = malloc(sizeof(Buffer) * 16); // TODO: Hacer de memoria dinamica
	for(int i = 0 ; i < 16; i++){
		 symat->buffer[i] = initBuffer();
	}
	symat->conf = malloc(sizeof(Conf));
	symat->conf->bufferpos = 0;
	symat->conf->posx = 0;
	symat->conf->posy = 0;
	symat->numtokens = getNumTokens(config);
	symat->tokens = getTokens(config,symat->numtokens);

}

// El span de y tiene que ser en funcion del ultimo token que este tiene.
void drawBuffer(Symat * symat, Buffer * buffer,float factor,int spanx,int spany,int * movex,int *prevhmax,bool firsttime,SDL_Color color){
	//FIXME: detectar en que mierda se rompe el buffer
	if(buffer == NULL){
		return;
	}
	if(buffer->tokensize == 0){
		return;
	}
	buffer->sizex =0;
	buffer->sizey =0;
	buffer->posx =0;
	buffer->posy =0;
	float dimfactor = (4*factor)/5;
	int charx, chary;
	int *supx = malloc(sizeof(int));
	int *subx = malloc(sizeof(int));
	*supx = 0;
	*subx = 0;
	if(buffer->tokens != NULL){
		SDL_Surface * chartest;
		if((buffer->tokens[buffer->tokensize-1].flags & ISBIGOP) == ISBIGOP){
			chartest = TTF_RenderUTF8_Solid(bigopfont,buffer->tokens[buffer->tokensize-1].token,color);
		} else {
			chartest = TTF_RenderUTF8_Solid(mainfont,buffer->tokens[buffer->tokensize-1].token,color);
		}
		if(chartest != NULL){
			charx = factor*chartest->w;
			chary = factor*chartest->h;
			SDL_FreeSurface(chartest);
		} else {
			charx = 0;
			chary = 0;
		}
	}
	drawLine(symat,buffer,buffer->tokens,buffer->tokensize,factor,spanx,spany,buffer->sizey,movex,prevhmax,color);
	// TODO: Añadir condicionales sobre bigop
	if(buffer->suptext != NULL)
		drawBuffer(symat,buffer->suptext,dimfactor,spanx+buffer->sizex-factor*3,spany,supx,NULL,false,color);
	if(buffer->subtext != NULL)
		drawBuffer(symat,buffer->subtext,dimfactor,spanx+buffer->sizex-factor*3,spany+(2*chary)/3,subx,NULL,false,color);
	if(*supx > *subx){
		*movex += *supx;
	} else {
		*movex += *subx;
	}
	if(buffer->next != NULL){
		drawBuffer(symat,buffer->next,factor,spanx+*movex,spany,movex,prevhmax,false,color);
	}
	free(subx);
	free(supx);
}

void drawLine(Symat * symat, Buffer * buffer, Token * tokens, int maxtokens,float factor,int spanx,int spany,int medy,int * movex,int *prevhmax,SDL_Color color){
	SDL_Texture * dummyTexture;
	SDL_Rect * rect = malloc(sizeof(SDL_Rect));
	int hmax = 0;
	if(prevhmax != NULL)
		hmax = *prevhmax;
	rect->y = spany;
	rect->x = spanx;
	for(int i = 0; i < maxtokens; i++){
		// TODO: Interpretacion de flags
		if(tokens[i].token != NULL){
			if((tokens[i].flags & ISBIGOP) == ISBIGOP){
				SDL_Surface * dummySurface = TTF_RenderUTF8_Solid(bigopfont,tokens[i].token,color);
				rect->w = dummySurface->w * factor;
				rect->y = spany;
				rect->h = dummySurface->h * factor;
				if(rect->h > hmax){
					hmax = rect->h;
				}
				dummyTexture = SDL_CreateTextureFromSurface(symat->renderer,dummySurface);
				SDL_RenderCopy(symat->renderer,dummyTexture,0,rect);
				rect->x = rect->x + dummySurface->w * factor;
				SDL_DestroyTexture(dummyTexture);
				SDL_FreeSurface(dummySurface);
			} else {
				SDL_Surface * dummySurface = TTF_RenderUTF8_Solid(mainfont,tokens[i].token,color);
				if(dummySurface == NULL){
					rect->w = 0;
					rect->y = spany;
					rect->h = 0;
				} else {
					rect->w = dummySurface->w * factor;
					rect->h = dummySurface->h * factor;
					if(hmax > 0){
					rect->y = spany + (hmax-rect->h)/2;
					} else {
					rect->y = spany;
					}
				}
				dummyTexture = SDL_CreateTextureFromSurface(symat->renderer,dummySurface);
				SDL_RenderCopy(symat->renderer,dummyTexture,0,rect);
				if(dummySurface != NULL){
					rect->x = rect->x + dummySurface->w * factor;
				}
				SDL_DestroyTexture(dummyTexture);
				SDL_FreeSurface(dummySurface);
			}
		}
	}
	buffer->sizex = rect->x-spanx;
	if(prevhmax != NULL)
		*prevhmax = hmax;
	if(movex != NULL)
		*movex = rect->x-spanx;
	buffer->sizey = hmax-spany;
	free(rect);
}

//TODO: Agregar mas detalle al printHelp
void printHelp(Symat * symat){
	static int greyish=0;
	static int helpdespy = 0;
	static int helpmaxy = 0;
	static int helpdespx = 0;
	int * helpmovex = malloc(sizeof(int));
	*helpmovex = 0;
	int * helpprevhmax = malloc(sizeof(int));
	*helpprevhmax = 0;
	SDL_Color white = {255,255,255,255};
	SDL_Color moving = {59,71,92,greyish+1};
	SDL_Rect helprect = {0,0,*symat->screenx,*symat->screeny};
	SDL_SetRenderDrawBlendMode(symat->renderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(symat->renderer,106,140,176,greyish);
	SDL_RenderFillRect(symat->renderer,&helprect);
	greyish += 5;
	if(greyish >= 200)
		greyish = 200;
	SDL_SetRenderDrawBlendMode(symat->renderer, 0);
	SDL_SetRenderDrawColor(symat->renderer,255,255,255,255);
	helpdespy +=6;
	int sinincx =200*sin((float)SDL_GetTicks()/800);
	for(int i = -200; i <= *symat->screenx; i += helpdespx){
		if(helpdespx == 0 || helpmaxy == 0){
			break;
		}
		sinincx = 200*sin((float)SDL_GetTicks()/800);
		for(int p = 0; p < *symat->screeny; p+= helpmaxy){
			drawBuffer(symat,symat->bufferhelp,1,i + sinincx,p,helpmovex,helpprevhmax,true,moving);
		}
	}
	drawBuffer(symat,symat->bufferhelp,1,0,*symat->screeny-helpdespy,helpmovex,helpprevhmax,true,white);
	helpmaxy = *helpprevhmax;
	if(helpdespy > helpmaxy)
		helpdespy = helpmaxy;
	helpdespx = *helpmovex;
	SDL_Surface * lovetext;
	lovetext = TTF_RenderUTF8_Solid(mainfont,"written with <3 by Capocha",white);
	helprect.x = *symat->screenx - 0.6*lovetext->w;
	helprect.y = *symat->screeny - 0.6*lovetext->h;
	helprect.h = 0.6*lovetext->h;
	helprect.w = 0.6*lovetext->w;
	SDL_Texture * dummyTexture;
	dummyTexture = SDL_CreateTextureFromSurface(symat->renderer,lovetext);
	SDL_RenderCopy(symat->renderer,dummyTexture,0,&helprect);
	SDL_FreeSurface(lovetext);
	SDL_DestroyTexture(dummyTexture);
	free(helpmovex);
	free(helpprevhmax);
	//SDL_SetRenderDrawBlendMode(symat->renderer, SDL_BLENDMODE_BLEND);
}

void triggerEasterEgg(Symat * symat,int n){
	switch(n){
		case 1:
			printHelp(symat);
			break;
	}
}
