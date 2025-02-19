#include "inc.h"

Symat symat;

//TODO: Hacer parser de latex a Symat
//TODO: Hacer parser de matrices
//TODO: Que puedas escribir en cualquier lado
//TODO TODO: Que cada buffer se guarde en una textura
//TODO TODO: Que se modifique cierta parte del buffer para no redibujar todo
//TODO: Que tenga geometría sintética
//TODO: Que dibuje gráficos de funciones en 2 y 3 dimensiones
//TODO: Hacer que los bigops se puedan dibujar como dos caracteres

int main(int argc, char* argv[]){
	SDL_Color black = {0,0,0,255};
	SDL_Color white = {255,255,255,255};
	SDL_Color nullcolor = {0,0,0,1};
	std::locale::global(std::locale("en_US.UTF-8"));

	if(GetErrors(&symat,argv[1])){
		return 1;
	}

    SDL_Event e;
    SDL_StartTextInput();
	bool neww = true;
	SDL_Texture * bufferdraw = nullptr;

	symat.buffer[0].parsetext = "a/int";
	while(!symat.quit){
        while(SDL_PollEvent(&e)){
            switch (e.type){
				case SDL_QUIT:
					symat.quit = true;
					break;
				case SDL_TEXTINPUT:
					//strcat(symat.buffer[0].parsetext,e.text.text);
					symat.buffer[0].parsetext += e.text.text;
					std::cout << symat.buffer[0].parsetext << '\n';
					break;
			}
			if(e.key.repeat == 1 || e.type == SDL_KEYDOWN){
				switch(e.key.keysym.sym){
					case SDLK_ESCAPE:
						symat.buffer[0].parsetext = "";
						break;
					case SDLK_BACKSPACE:
						remove_last_utf8_char(symat.buffer[0].parsetext);
						break;
				}
			}
		}

		readTokens(symat,symat.buffer[0]);
		SDL_SetRenderDrawColor(symat.renderer,255,255,255,255);
		SDL_RenderPresent(symat.renderer);
		SDL_RenderClear(symat.renderer);
		*symat.movex = 0;
		*symat.prevhmax = 0;
		drawBuffer(symat,symat.buffer[0],1,100,100,symat.movex,symat.prevhmax,true,black,nullcolor,true);
		drawParse(symat,symat.buffer[0],black,white);
		/*
		if(strcmp(symat.buffer[0].parsetext,"about") == 0){
			triggerEasterEgg(&symat,1);
		}
		*/
		SDL_GetWindowSize(symat.window, symat.screenx, symat.screeny);
		SDL_Delay(60);
	}
	SDL_DestroyTexture(bufferdraw);
	return 0;
}
