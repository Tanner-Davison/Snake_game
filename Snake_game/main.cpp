#include <iostream>
#include <conio.h>
#include <sys/timeb.h>
#include <time.h>
#include <deque>

using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;
const time_t TICK_MS = 500;


enum class Action { Stop, Continue, Up, Down, Left, Right };

static time_t GetCurrentTimeMS() {
	_timeb current_time = { 0 };
	_ftime_s(&current_time);
	return current_time.time * 1000 + current_time.millitm;
}


class Field {
public:
	Field() {
		for (int y = 0; y < HEIGHT; y++) {
			bool isFirstRow = (y == 0);
			bool isLastRow = (y == (HEIGHT - 1));
			for (int x = 0; x < WIDTH; x++) {
				bool isFisrtColumn = (x == 0);
				bool isLastColumn = (x == (WIDTH - 1));
				if (isFirstRow || isLastRow) chars[y][x] = '-';
				else if (isFisrtColumn || isLastColumn) chars[y][x] = '|';
				else chars[y][x] = ' ';
			}
		}
		fruit = NULL;
		zmeika = NULL;
	}

	~Field() {
		if (fruit != NULL) {
			delete fruit; fruit = NULL;
		}
		if (zmeika != NULL) {
			delete zmeika; zmeika = NULL;
		}
	}

	void Draw() {
		system("cls");
		for (int y = 0; y < HEIGHT; y++)
		{
			cout.write(chars[y], WIDTH) << endl;
		}
		cout << "Fruits: " << GetScore();
	}

	void AddZmeikaAndFruit() {
		if (zmeika == NULL) {
			AddZmeika();
		}
		if (fruit == NULL) {
			AddFruit();
		}
	}

	bool PerformZmeikaAction(Action zmeikaAction) {
		if (zmeikaAction == Action::Stop)
			return true;

		bool success_move = zmeika->MoveHead(zmeikaAction);
		BodyCell head = zmeika->GetHead();

		if (success_move == false) {
			chars[head.Y][head.X] = 'X';
			return true;
		}

		if (head.Y == fruit->Y && head.X == fruit->X) {
			zmeika->EatFruit();
			AddFruit();
		}
		else {
			BodyCell tail = zmeika->CutTail();
			chars[tail.Y][tail.X] = ' ';
			if (IsBorder(head.Y, head.X)) {
				chars[head.Y][head.X] = 'X';
				return true;
			}
		}

		chars[head.Y][head.X] = head.Symbol;
		return false;
	}

	int GetScore() {
		if (zmeika == NULL) {
			return 0;
		}
		return zmeika->EatenFruits();
	}


private:

	void AddZmeika() {
		zmeika = new Zmeika(HEIGHT / 2, WIDTH / 2);
		BodyCell head = zmeika->GetHead();
		chars[head.Y][head.X] = head.Symbol;
	}

	void AddFruit() {
		if (fruit != NULL) {
			delete fruit; fruit = NULL;
		}
		int fy, fx;
		do {
			fx = rand() % WIDTH; fy = rand() % HEIGHT;
		} while (IsBorder(fy, fx) || zmeika->IsBody(fy, fx));
		fruit = new Fruit(fy, fx);
		chars[fruit->Y][fruit->X] = fruit->Symbol;
	}

	bool IsBorder(int y, int x) {
		return (y == 0) || (y == HEIGHT - 1) || (x == 0) || (x == WIDTH - 1);
	}

	struct BodyCell {
		int Y;
		int X;
		char Symbol;

		BodyCell(int y, int x) {
			Y = y; X = x; Symbol = '@';
		}
	};

	class Zmeika {
	public:
		Zmeika(int y, int x) {
			currentAction = Action::Down;
			BodyCell head(y, x);
			body.push_front(head);
			fruits = 0;
		}

		bool IsBody(int y, int x) {
			for (BodyCell cell : body) {
				if ((cell.Y == y) && (cell.X == x))
					return true;
			}
			return false;
		}

		BodyCell GetHead() {
			return body.front();
		}

		BodyCell CutTail() {
			BodyCell tail = body.back();
			body.pop_back();
			return tail;
		}

		bool MoveHead(Action action) {
			BodyCell head = body.front();
			if (action == Action::Continue)
				action = currentAction;
			else
				currentAction = action;
			BodyCell new_head(head.Y, head.X);
			switch (action)
			{
			case Action::Down: new_head.Y += 1; break;
			case Action::Up: new_head.Y -= 1; break;
			case Action::Right: new_head.X += 1; break;
			case Action::Left: new_head.X -= 1; break;
			}
			if (IsBody(new_head.Y, new_head.X))
				return false;
			body.push_front(new_head);
			return true;
		}

		void EatFruit() {
			fruits++;
		}

		int EatenFruits() {
			return fruits;
		}

	private:

		deque<BodyCell> body;
		Action currentAction;
		int fruits;
	};

	class Fruit {
	public:
		int Y;
		int X;
		char Symbol;

		Fruit(int y, int x) {
			Y = y; X = x; Symbol = '*';
		}
	};

	char chars[HEIGHT][WIDTH] = { 0 };
	Zmeika* zmeika;
	Fruit* fruit;
};

static Action GetActionFromUser(time_t milliseconds_to_wait) {
	time_t start_milliseconds = GetCurrentTimeMS();

	do {
		while (0 == _kbhit()) {
			time_t elapsed_milliseconds = GetCurrentTimeMS() - start_milliseconds;
			if (elapsed_milliseconds >= milliseconds_to_wait)
				return Action::Continue;
		}
		switch (_getch()) {
		case 13:
			return Action::Stop;
		case 0:
		case 0xE0:
			switch (_getch()) {
			case 75:
				return Action::Left;
			case 77:
				return Action::Right;
			case 72:
				return Action::Up;
			case 80:
				return Action::Down;
			}
		}
	} while (true);
}


int main()
{
	Field field;
	field.AddZmeikaAndFruit();
	field.Draw();

	bool gameOver = false;
	while (gameOver == false) {
		Action action = GetActionFromUser(TICK_MS);
		gameOver = field.PerformZmeikaAction(action);
		field.Draw();
	}
	cout << endl << "Game over!" << endl;
}