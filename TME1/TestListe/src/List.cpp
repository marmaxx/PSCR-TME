// FAUTE : il manque le include de "List.h" (faute syntaxique)
#include "List.h"
namespace pr {

// ******************* Chainon
Chainon::Chainon (const std::string & data, Chainon * next):data(data),next(next) {};

size_t Chainon::length() {
	size_t len = 1;
	if (next != nullptr) {
		len += next->length();
	}
	//FAUTE : il faut return len (faute grave)
	return len;
}

//FAUTE : oubli de "const" pour la fonction print (faute syntaxique)
void Chainon::print (std::ostream & os) const{
	os << data ;
	if (next != nullptr) {
		os << ", ";
		// FAUTE : il faut next->print(os) dans le if (faute grave)
		next->print(os);
	}
}

// ******************  List
const std::string & List::operator[] (size_t index) const  {
	Chainon * it = tete;
	for (size_t i=0; i < index ; i++) {
		it = it->next;
	}
	return it->data;
}

void List::push_back (const std::string& val) {
	if (tete == nullptr) {
		tete = new Chainon(val);
	} else {
		Chainon * fin = tete;
		while (fin->next) {
			fin = fin->next;
		}
		fin->next = new Chainon(val);
	}
}

// FAUTE : redéfinition de push_front (faute logique)
/*void List::push_front (const std::string& val) {
	tete = new Chainon(val,tete);
}*/

// FAUTE : oubli de List:: avant empty (faute syntaxique)
bool List::empty() {
	return tete == nullptr;
}

size_t List::size() const {
	if (tete == nullptr) {
		return 0;
	} else {
		return tete->length();
	}
}

//FAUTE : operator n'était pas dans le namespace pr (faute syntaxique) 
std::ostream & operator<< (std::ostream & os, const pr::List & vec)
{
	os << "[";
	if (vec.tete != nullptr) {
		vec.tete->print (os) ;
	}
	os << "]";
	return os;
}

} // namespace pr