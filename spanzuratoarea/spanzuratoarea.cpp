#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

// Functie pentru alegerea unui cuvant aleatoriu din lista
string chooseWord(const vector<string>& words) {
    srand(time(0));
    int index = rand() % words.size();
    return words[index];
}

// Functie pentru afisarea cuvantului cu litere ascunse ca underscore-uri
string displayWord(const string& word, const vector<bool>& revealed) {
    string display = "";
    for (size_t i = 0; i < word.length(); ++i) {
        if (revealed[i]) {
            display += word[i];
        } else {
            display += "_";
        }
        display += " ";
    }
    return display;
}

// Functie pentru verificarea daca litera este in cuvant
bool checkLetter(const string& word, vector<bool>& revealed, char letter, int& lives) {
    bool found = false;
    for (size_t i = 0; i < word.length(); ++i) {
        if (tolower(word[i]) == tolower(letter)) {
            revealed[i] = true;
            found = true;
        }
    }
    if (!found) {
        lives--;
    }
    return found;
}

int main() {
    // Lista de cuvinte romanesti
    vector<string> words = {"casa", "masina", "calculator", "programare", "soare", "luna", "apa", "foc", "pamant", "cer"};

    bool playAgain = true;
    while (playAgain) {
        // Curatarea ecranului
        system("cls");
        
        // Alegerea cuvantului
        string word = chooseWord(words);

        // Vector pentru literele dezvaluite
        vector<bool> revealed(word.length(), false);

        // Vector pentru literele folosite
        vector<char> usedLetters;

        // Vieti initiale
        int lives = 6;

        cout << "Bine ati venit la Spanzuratoarea!" << endl;
        cout << "Ghiciti cuvantul romanesc." << endl;
        cout << "Vieti resetate la: " << lives << endl << endl;

        // Bucla principala a jocului
        while (lives > 0) {
            // Afisarea desenului spanzuratorii
            cout << endl;
            cout << "  +---+" << endl;
            cout << "  |   |" << endl;
            if (lives < 6) cout << "  O   |" << endl; else cout << "      |" << endl;
            if (lives < 4) cout << " /"; else cout << " ";
            if (lives < 5) cout << "|"; else cout << " ";
            if (lives < 3) cout << "\\"; else cout << " ";
            cout << "  |" << endl;
            if (lives < 2) cout << " /"; else cout << "  ";
            if (lives < 1) cout << " \\"; else cout << "  ";
            cout << " |" << endl;
            cout << "      |" << endl;
            cout << "=========" << endl;

            // Afisarea cuvantului
            cout << "Cuvant: " << displayWord(word, revealed) << endl;

            // Afisarea vietilor ramase
            cout << "Vieti ramase: " << lives << endl;

            // Afisarea literelor folosite
            cout << "Litere folosite: ";
            for (char c : usedLetters) {
                cout << c << " ";
            }
            cout << endl;

            // Verificarea daca cuvantul a fost ghicit
            bool guessed = true;
            for (bool b : revealed) {
                if (!b) {
                    guessed = false;
                    break;
                }
            }
            if (guessed) {
                cout << "Felicitari! Ati ghicit cuvantul: " << word << endl;
                break;
            }

            // Citirea literei de la utilizator
            cout << "Introduceti o litera: ";
            char letter;
            cin >> letter;
            letter = tolower(letter);

            // Verificarea daca litera a fost deja folosita
            bool alreadyUsed = false;
            for (char c : usedLetters) {
                if (c == letter) {
                    alreadyUsed = true;
                    break;
                }
            }
            if (alreadyUsed) {
                cout << "Litera deja folosita!" << endl;
                continue;
            }

            // Adaugarea literei la cele folosite
            usedLetters.push_back(letter);

            // Verificarea literei
            if (!checkLetter(word, revealed, letter, lives)) {
                cout << "Litera gresita!" << endl;
            }
        }

        if (lives == 0) {
            // Joc pierdut
            cout << endl;
            cout << "  +---+" << endl;
            cout << "  |   |" << endl;
            cout << "  O   |" << endl;
            cout << " /|\\  |" << endl;
            cout << " / \\  |" << endl;
            cout << "      |" << endl;
            cout << "=========" << endl;
            cout << "Ati pierdut! Cuvantul era: " << word << endl;
        }

        // Intrebare pentru reincepere
        cout << "Doriti sa jucati din nou? (y/n): ";
        char choice;
        cin >> choice;
        if (tolower(choice) != 'y') {
            playAgain = false;
        }
    }

    return 0;
}