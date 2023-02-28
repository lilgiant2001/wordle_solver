# wordle_solver
Program that guesses the word that user inputs  (like a person playing the wordle game)

To use our wordle solver:
1. Run make in the terminal
2. When prompted by the program, enter a word length between 3 and 8
3. Choose either option 1 or 2
4. If option 1 is chosen, the program will generate a random word for the user to guess and it becomes a regular wordle game.
5. If option 2 is chosen, meaning letting the program solve a word typed in by the user, the program will let the user choose between two solving modes: 1. random candidate or 2. statistically optimal candidate
6. If random candidate is chosen, the program just picks candidate from the correspondent wordlist randomly. If statistically optimal candidate is chosen, the program picks the candidate based on our scoring algorithm.
7. Lastly, the program prints out all the guesses it made and the correct answer.

Example:
----------------------------------
|  Welcome to our wordle solver!  |
----------------------------------

What is your word length?

Answer: 5

Do you want to play a game?
1. Yes
2. No

Answer: 2

What is your word?

Answer: apple

Which algorithm do you want to use?
1. Random Candidate
2. Statistically Optimal Candidate

Answer: 2

tares

peace

apple

Done.
