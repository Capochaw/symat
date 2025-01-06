#include "inc.h"
#include <stdio.h>

Symat symat;

int main(int argc, char * argv[]){
	SDL_Color black = {0,0,0,255};
	SDL_Color white = {255,255,255,255};
	if(GetErrors(&symat,argv[1])){
		return 1;
	}
	Token * toprint = malloc(sizeof(Token) * 16);

    SDL_Event e;
    SDL_StartTextInput();
	//strcpy(symat.bufferhelp->parsetext,"intyM nablaτ");
	//readTokens(&symat,symat.bufferhelp);

	while(!symat.quit){
        while(SDL_PollEvent(&e)){
            switch (e.type){
				case SDL_QUIT:
					symat.quit = true;
					break;
				case SDL_TEXTINPUT:
					strcat(symat.buffer[0].parsetext,e.text.text);
					break;
			}
			if(e.key.repeat == 1 || e.type == SDL_KEYDOWN){
				switch(e.key.keysym.sym){
					case SDLK_ESCAPE:
						strcpy(symat.buffer[0].parsetext,"");
						break;
					case SDLK_BACKSPACE:
						removeLastCharacter(symat.buffer[0].parsetext);
						break;
				}
			}
		}

		SDL_SetRenderDrawColor(symat.renderer,255,255,255,255);
		readTokens(&symat,&symat.buffer[0]);
		SDL_RenderClear(symat.renderer);
		*symat.movex = 0;
		*symat.prevhmax = 0;
		drawBuffer(&symat,&symat.buffer[0],1,100,100,symat.movex,symat.prevhmax,true,black);
		if(strcmp(symat.buffer[0].parsetext,"about") == 0){
			triggerEasterEgg(&symat,1);
		}
		SDL_RenderPresent(symat.renderer);
		UpdateSymat(&symat);
		SDL_Delay(60);
	}
	return 0;
}
