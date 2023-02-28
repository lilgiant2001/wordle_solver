CC := clang
CFLAGS := -g -Wall -Werror -fsanitize=address

all: wordlesolver

clean:
	rm -f wordlesolver

wordlesolver: wordlesolver.c wordlist_helper.h find_candidate.c check_validity.c filter_wordlist.c data_generator.h data_generator.c ui.h ui.c
	$(CC) $(CFLAGS) -o wordlesolver wordlesolver.c find_candidate.c check_validity.c filter_wordlist.c data_generator.c ui.c -lpthread

zip:
	@echo "Generating wordlesolver.zip file to submit to Gradescope..."
	@zip -q -r wordlesolver.zip . -x .git/\* .vscode/\* .clang-format .gitignore wordlesolver
	@echo "Done. Please upload wordlesolver.zip to Gradescope."

format:
	@echo "Reformatting source code."
	@clang-format -i --style=file $(wildcard *.c) $(wildcard *.h)
	@echo "Done."

.PHONY: all clean zip format
