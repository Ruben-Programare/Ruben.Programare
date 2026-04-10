#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main()
{
    int randomnumber, attempt, maxattempts = 10, tries = 0;

    srand(time(0));

    randomnumber = rand() % 101;

    cout << "Guess the number (0-100). You have " << maxattempts << " tries" << endl;

    while(tries < maxattempts){
        cin >> attempt;
        tries++;

        if(attempt == randomnumber){
            cout << "Congrats, you guessed the number in " << tries << " tries" << endl;
            return 0;
        }
        else{
            if(attempt > randomnumber)
                cout << "number is too big. You have " << (maxattempts - tries) << " left" << endl;
            else
                cout << "number is too low. You have " << (maxattempts - tries) << " left" << endl;
        }
    }

    cout << "Game over! Number was: " << randomnumber << endl;

    return 0;
}