#include "inc.h"

int getNumTokens(const std::string & config){
    std::ifstream file(config);
    if (!file.is_open()) {
        throw std::runtime_error("No config file");
    }
	int numtokens = 0;
	std::string line;
	while(std::getline(file,line)){
		numtokens++;
	}
	return numtokens;
}

std::string cutParse(const std::string& source, long unsigned int i) {
    if (i >= source.size()) {
        return ""; // Retorna una cadena vacía si el índice está fuera de rango
    }
    return source.substr(i); // Devuelve una subcadena desde el índice `i` hasta el final
}

std::string getBbparse(Symat& symat, const std::string& source, bool issup, long unsigned int* retpos) {
    std::string ret;
    bool broke = false;
    long unsigned int size = 0, move, dim = 0, cant = 0;

    if (retpos != nullptr) {
        move = *retpos + 1;
    } else {
        move = 1;
    }

    if (issup) {
        dim += 1;
        while ((move + size) < source.size() && !broke) {
            ret += source[move + size];
            size++;

            if (ret.back() == '^' && ret[ret.size() - 2] != '^') {
                dim += 1;
            }
            if (ret.back() == '_' && ret[ret.size() - 2] == '_') {
                dim -= 1;
                cant++;
            }
            if (ret.back() == '_' && ret[ret.size() - 2] == '_' && dim == 0) {
                broke = true;
                if (cant % 2 == 0) {
                    ret.pop_back();
                } else {
                    ret.pop_back();
                    ret.pop_back();
                }
                if (retpos != nullptr) {
                    *retpos += size + 1;
                }
            }
        }
    } else {
        dim -= 1;
        while ((move + size) < source.size() && !broke) {
            ret += source[move + size];
            size++;

            if (ret.back() == '^' && ret[ret.size() - 2] == '^') {
                dim += 1;
                cant++;
            }
            if (ret.back() == '_' && ret[ret.size() - 2] != '_') {
                dim -= 1;
            }
            if (ret.back() == '^' && ret[ret.size() - 2] == '^' && dim == 0) {
                broke = true;
                if (cant % 2 == 0) {
                    ret.pop_back();
                } else {
                    ret.pop_back();
                    ret.pop_back();
                }
                if (retpos != nullptr) {
                    *retpos += size + 1;
                }
            }
        }
    }

    if (cant % 2 == 0) {
        if (!ret.empty()) ret.pop_back();
    } else {
        if (ret.size() > 1) ret.pop_back();
    }

    if (retpos != nullptr && !broke) {
        *retpos += size + 1;
    }

    return ret;
}

Token getTokenFlags(std::string & s,int & pos){
	Token ret;
	bool close = false;
	int i = 0;
	ret.flags = 0;
	while(s[i] != ')'){
		i++;
	}
	while(s[i] != '('){
		switch(s[i]){
			case 'B':
				ret.flags |= ISBIGOP;
				break;
			case 'P':
				ret.flags |= ISPARENTHESIS;
				if(close){
					ret.flags |= CLOSEPAR;
				}
				close = true;
				break;
			case 'F':
				ret.flags |= ISFRAC;
				break;
			case 'U':
				ret.flags |= MAKESUP;
				break;
			case 'u':
				ret.flags |= MAKESUB;
				break;
		}
		i++;
	}
	pos = i+2;
	return ret;
}

// TODO: Mejorar la estructura de datos para que clasifique en bigops, fracs y parentesis
std::vector<Token> getTokens(const char* config, int numtokens) {
    std::ifstream file(config);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file");
    }

    std::vector<Token> ret(numtokens);
    std::string actline;
    for (int i = 0; i < numtokens; i++) {
        std::getline(file, actline);
        int pos = 0;

        // Asume que `getTokenFlags` devuelve un Token válido
        ret[i] = getTokenFlags(actline, pos);

        std::string chara, token, num;
        bool isFirstElement = true, isSecondElement = false;

        for (size_t p = pos; p < actline.length(); p++) {
            if (isFirstElement) {
                if (actline[p] != '"') {
                    chara += actline[p];
                } else {
                    isFirstElement = false;
                    isSecondElement = true;
                    p += 2;
                }
            } else if (isSecondElement) {
                if (actline[p] != '"') {
                    token += actline[p];
                } else {
                    isSecondElement = false;
                    p += 2;
                }
            } else {
                num += actline[p];
            }
        }

        ret[i].chara = chara;
        ret[i].token = token;
    }

    file.close();
    return ret;
}
void remove_last_utf8_char(std::string& str) {
    if (str.empty()) return; // Si la cadena está vacía, no hacemos nada

    // Recorre desde el final para encontrar el inicio del último carácter
    size_t i = str.size() - 1;

    // Retrocede hasta encontrar el byte inicial del carácter (no 10xxxxxx)
    while (i > 0 && (str[i] & 0xC0) == 0x80) {
        --i; // Byte de continuación encontrado, sigue retrocediendo
    }

    // Recorta la cadena eliminando el último carácter
    str.erase(i);
}

std::string checkParenthesis(const std::string& source,Buffer &dest, bool set,size_t *pos){
	std::string comp;
	size_t move = 0,diff = 0;
	int counter = 0;
	if(set){
		while(source[diff] != '/'){
			if(source[diff] == '(' && counter == 0){
				move = diff + 1;
				break;
			} else if (source[diff] == ')' && counter != 0){
				counter -=1;
			}
			if(source[diff] == ')'){
				counter +=1;
			}
			diff++;
		}
		for(size_t i = 0; i < dest.tokens.size();i++){
			if(dest.tokens[i].chara == "("){
				dest.tokens.erase(dest.tokens.begin()+i);
			}
		}
		for(size_t i = move; i < source.size(); i++){
			if(source[i] == '/'){
				break;
			}
			comp += source[i];
		}
	} else {
		while(source[move] != '/'){
			move++;
		}
		for(size_t i = move + 1; i < source.size(); i++){
			comp += source[i];
			if(source[i] == ')' && counter == 0){
				comp.pop_back();
				*pos = i;
				break;
			} else if(source[i] == ')' && counter != 0){
				counter -= 1;
			}
			if(source[i] == '('){
				counter += 1;
			}
			if(i == source.size()-1){
				*pos = i;
				break;
			}
		}
	}
	return comp;
}

Token auxToken(const std::string& text) {
    Token ret;
    ret.flags = 0;
    ret.token = text;        // Asignamos directamente el texto a la cadena.
    ret.chara = ret.token;   // `chara` puede ser un puntero al inicio de la cadena si sigue siendo relevante.
    return ret;
}

int isToken(std::string& prev, std::string& comp, const std::vector<Token>& tokens, bool* useprev) {
    int ret = -1;
    std::string move;

    for (size_t i = 0; i < comp.size(); ++i) {
        move = comp.substr(i);  // Extrae la subcadena desde el índice `i`.

        for (size_t p = 0; p < tokens.size(); ++p) {
            if (move == tokens[p].chara) {
                ret = p;

                // Actualiza `prev` con la parte inicial de `comp` no incluida en `move`.
                prev = comp.substr(0, comp.size() - move.size());

                if (useprev) {
                    *useprev = true;
                }

                // Actualiza `comp` para que solo contenga `move`.
                comp = move;
                return ret;
            }
        }
    }

    return ret;
}

// En función de '/' y '('
void removeTokens(Buffer &dest, Buffer& comp){
	for(size_t i = dest.tokens.size(); i-- > 0;){
		for(size_t p = 0; p < comp.tokens.size(); p++){
			if(dest.tokens[i].token == comp.tokens[p].token && dest.tokens[i].chara == comp.tokens[p].chara && dest.tokens[i].flags == comp.tokens[p].flags){
				dest.tokens.erase(dest.tokens.begin()+i);
				break;
			}
		}
	}
}

void readTokens(Symat& symat, Buffer& dest) {
    dest.tokens.clear();
    dest.size = {0, 0};
    dest.span = {0, 0};
    dest.subtext.reset();
    dest.suptext.reset();
    dest.fracsubtext.reset();
    dest.fracsuptext.reset();
    dest.next.reset();

    std::string& source = dest.parsetext;
    std::string prev, comp;
    int p = 0, pos = 0;
    bool hasTokenPrev = false, usePrev = false;

    for (size_t i = 0; i < source.size(); ++i) {
        comp += source[i];
        int tokenPos = isToken(prev, comp, symat.tokens, &usePrev);

        if (tokenPos != -1) {  // Si es un token
            const auto& token = symat.tokens[tokenPos];
            if ((token.flags & MAKESUB) || (token.flags & MAKESUP)) {
                if (token.flags & MAKESUB) {
                    dest.subtext = std::make_unique<Buffer>();
                    dest.subtext->parsetext = getBbparse(symat, source, false, &i);
                    readTokens(symat, *dest.subtext);

                    prev.clear();
                    comp = source[i];
                    tokenPos = isToken(prev, comp, symat.tokens,nullptr);

                    if (symat.tokens[tokenPos].flags & MAKESUP) {
                        dest.suptext = std::make_unique<Buffer>();
                        dest.suptext->parsetext = getBbparse(symat, source, true, &i);
                        if (!dest.suptext->parsetext.empty()) {
                            readTokens(symat, *dest.suptext);
                        }
                    }
                }

                if (token.flags & MAKESUP) {
                    dest.suptext = std::make_unique<Buffer>();
                    dest.suptext->parsetext = getBbparse(symat, source, true, &i);
                    readTokens(symat, *dest.suptext);

                    prev.clear();
                    comp = source[i];
                    tokenPos = isToken(prev, comp, symat.tokens, nullptr);

                    if (symat.tokens[tokenPos].flags & MAKESUB) {
                        dest.subtext = std::make_unique<Buffer>();
                        dest.subtext->parsetext = getBbparse(symat, source, false, &i);
                        if (!dest.subtext->parsetext.empty()) {
                            readTokens(symat, *dest.subtext);
                        }
                    }
                }

                dest.next = std::make_unique<Buffer>();
                dest.next->parsetext = cutParse(source, i);
                if (!dest.next->parsetext.empty()) {
                    readTokens(symat, *dest.next);
                }
                break;
            }
            if (dest.tokens.empty()) {
				if(token.flags & ISFRAC){
					remove_last_utf8_char(source);
					continue;
				}
                dest.tokens.push_back(symat.tokens[tokenPos]);
                pos++;
                p = 0;
                comp.clear();
            } else {
				// FIXME: NO es capaz de leer expresiones anidadas
				// FIXME: Las expresiones anidadas agrega elementos que no tienen que ir.
				if(token.flags & ISFRAC){
					//Esta función entrega la posición donde termina la fracción y entrega 2
					//vectores de tokens. Uno con la parte de arriba y la otra con la parte
					//de abajo de la fracción.
					dest.fracsuptext = std::make_unique<Buffer>();
					dest.fracsubtext = std::make_unique<Buffer>();
					dest.fracsuptext->parsetext = checkParenthesis(source,dest,true,&i); //parte de arriba
					dest.fracsubtext->parsetext = checkParenthesis(source,dest,false,&i);
					readTokens(symat, *dest.fracsuptext);
					readTokens(symat, *dest.fracsubtext);
					dest.tokens.resize(dest.tokens.size() -dest.fracsuptext->tokens.size());
					//removeTokens(dest,*dest.fracsuptext);
					//removeTokens(dest,*dest.fracsubtext);
					prev.clear();
					comp.clear();
					//TODO: Quitar todos los tokens similares a los de checkParentesis
                	dest.next = std::make_unique<Buffer>();
                	dest.next->parsetext = cutParse(source, i+1);
                	if (!dest.next->parsetext.empty()) {
                	    readTokens(symat, *dest.next);
                	}
					return;
				}
                if (usePrev) {
					if(!(dest.tokens.back().flags & ISPARENTHESIS))
                    	dest.tokens.pop_back();
                    dest.tokens.push_back(auxToken(prev));
                    pos++;
                    usePrev = false;
                }
				if(!(symat.tokens[tokenPos].flags & ISFRAC))
                	dest.tokens.push_back(symat.tokens[tokenPos]);
                p = 0;
                comp.clear();
            }
            hasTokenPrev = true;
        } else {  // Si no es un token
            if (dest.tokens.empty()) {
                dest.tokens.push_back(auxToken(comp));
                pos++;
            } else {
                if (hasTokenPrev) {
                    dest.tokens.push_back(auxToken(comp));
                    pos++;
                    hasTokenPrev = false;
                } else {
                    dest.tokens.back() = auxToken(comp);
                }
            }
        }
    }
}
