#include <SFML/Graphics.hpp>
#include <sstream>

constexpr size_t MARK_SIZE_PX = 160;

constexpr size_t FONT_SIZE = 30;

constexpr float GRID_X = 138;
constexpr float GRID_Y = 140;

constexpr float CELL_SIZE_PX = 160;
constexpr float CELL_OFFSET_PX = 24;

sf::Vector2f getDrawPositionFromGridPosition(size_t row, size_t col)
{
	const auto x = GRID_X + col * (CELL_SIZE_PX + CELL_OFFSET_PX);
	const auto y = GRID_Y + row * (CELL_SIZE_PX + CELL_OFFSET_PX);
	return { x, y };
}

sf::Vector2i getGridPositionFromDrawPosition(float x, float y)
{
	if (x < GRID_X || x > GRID_X + 3 * (CELL_SIZE_PX + CELL_OFFSET_PX) ||
		y < GRID_Y || y > GRID_Y + 3 * (CELL_SIZE_PX + CELL_OFFSET_PX))
	{
		return { -1, -1 };
	}

	const int row = (y - GRID_Y) / (CELL_SIZE_PX + CELL_OFFSET_PX);
	const int col = (x - GRID_X) / (CELL_SIZE_PX + CELL_OFFSET_PX);

	return { row, col };
}

int getWinningPlayer(int grid[3][3])
{
	// Check diagonals
	bool mainDiagonalHasWinner = true;
	bool secondaryDiagonalHasWinner = true;
	for (int i = 1; i < 3; ++i)
	{
		mainDiagonalHasWinner = mainDiagonalHasWinner &&
			grid[i][i] != -1 && grid[i][i] == grid[i - 1][i - 1];
		secondaryDiagonalHasWinner = secondaryDiagonalHasWinner &&
			grid[i][2 - i] != -1 && grid[i][2 - i] == grid[i - 1][2 - i + 1];
	}
	if (mainDiagonalHasWinner || secondaryDiagonalHasWinner)
	{
		return grid[1][1];
	}

	// Check rows and cols
	for (int i = 0; i < 3; ++i)
	{
		bool rowHasWinner = true;
		bool colHasWinner = true;
		for (int j = 1; j < 3; ++j)
		{
			rowHasWinner = rowHasWinner &&
				grid[i][j] != -1 && grid[i][j - 1] == grid[i][j];
			colHasWinner = colHasWinner &&
				grid[j][i] != -1 && grid[j - 1][i] == grid[j][i];
		}
		if (rowHasWinner || colHasWinner)
		{
			return grid[i][i];
		}
	}

	return -1;
}

bool hasValidMove(int grid[3][3])
{
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (grid[i][j] == -1)
			{
				return true;
			}
		}
	}

	return false;
}

struct BestMove
{
	int value = 0;
	int row = -1;
	int col = -1;
};

constexpr int MAX = 0;
constexpr int MIN = 1;
constexpr int EMPTY = -1;

BestMove minimax(int grid[3][3], bool isMax, int depth, int alpha, int beta)
{
	const int winner = getWinningPlayer(grid);
	if (winner == MAX)
	{
		return { 10 - depth, -1, -1 };
	}
	else if (winner == MIN)
	{
		return { depth - 10, -1, -1 };
	}

	if (!hasValidMove(grid))
	{
		return { 0, -1, -1 };
	}

	BestMove bestMove{ isMax ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max(), -1, -1 };

	// Generate next move
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			if (grid[i][j] == EMPTY)
			{
				grid[i][j] = isMax ? MAX : MIN; // temporary fill the cell with the current player's mark

				const bool isNextPlayerMax = !isMax;
				auto currentMove = minimax(grid, isNextPlayerMax, depth + 1, alpha, beta);

				grid[i][j] = EMPTY; // undo move

				if (isMax && currentMove.value > bestMove.value || 
					!isMax && currentMove.value < bestMove.value)
				{
					bestMove = currentMove;
					bestMove.row = i;
					bestMove.col = j;
				}

				if (isMax)
				{
					alpha = std::max(alpha, bestMove.value);
				}
				else
				{
					beta = std::min(beta, bestMove.value);
				}

				if (beta <= alpha)
				{
					return bestMove;
				}
			}
		}
	}

	return bestMove;
}

void WinMain()
{
	sf::RenderWindow window(sf::VideoMode(800, 800), "Tic-tac-toe");

	sf::Texture boardTexture;
	boardTexture.loadFromFile("Assets/Board.png");

	sf::Sprite board{ boardTexture };

	sf::Texture marksTexture;
	marksTexture.loadFromFile("Assets/Marks.png");

	sf::Sprite markX{ marksTexture };
	markX.setTextureRect({ 0, 0, MARK_SIZE_PX, MARK_SIZE_PX });

	sf::Sprite markO{ marksTexture };
	markO.setTextureRect({ MARK_SIZE_PX, 0, MARK_SIZE_PX, MARK_SIZE_PX });

	sf::Font font;
	font.loadFromFile("Assets/arial.ttf");

	sf::Text gameOver{ "", font, FONT_SIZE };
	gameOver.setPosition(GRID_X, GRID_Y - 4 * CELL_OFFSET_PX);

	int grid[3][3];
	std::memset(grid, -1, sizeof(grid)); // set all positions to -1

	int currentPlayer = 0;
	bool isGameOver = false;

	auto evaluateGameOver = [&]()
	{
		int winningPlayer = getWinningPlayer(grid);
		if (winningPlayer != EMPTY)
		{
			isGameOver = true;

			std::ostringstream oss;
			oss << "Player " << winningPlayer
				<< (winningPlayer == 0 ? " (mark X) " : " (mark O) ")
				<< "wins\n"
				<< "Press F5 for new game!";
			gameOver.setString(oss.str());
		}
		// Handle draw result
		else if (!hasValidMove(grid))
		{
			isGameOver = true;

			std::ostringstream oss;
			oss << "Draw!\n"
				<< "Press F5 for new game!";
			gameOver.setString(oss.str());
		}
	};

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
			{
				window.close();
			}
			// Handle clicks
			else if (!isGameOver && event.type == sf::Event::MouseButtonReleased &&
				event.mouseButton.button == sf::Mouse::Button::Left)
			{
				const auto pos = getGridPositionFromDrawPosition(event.mouseButton.x, event.mouseButton.y);
				if (pos.x != -1 && pos.y != -1 && grid[pos.x][pos.y] == -1/*is free position*/)
				{
					grid[pos.x][pos.y] = currentPlayer;
					currentPlayer = !currentPlayer;

					evaluateGameOver();
					if (!isGameOver)
					{
						// Make AI move
						BestMove move = minimax(grid, false, 0, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
						grid[move.row][move.col] = MIN;
						currentPlayer = !currentPlayer;
						evaluateGameOver();
					}
				}
			}
			// Start new game
			else if (isGameOver && event.type == sf::Event::KeyReleased &&
				event.key.code == sf::Keyboard::F5)
			{
				isGameOver = false;
				gameOver.setString("");
				std::memset(grid, -1, sizeof(grid)); // set all positions to -1
				currentPlayer = 0;
			}
		}

		window.clear();
		window.draw(board);

		// Draw placed marks
		for (int i = 0; i < 3; ++i)
		{
			for (int j = 0; j < 3; ++j)
			{
				if (grid[i][j] == -1)
				{
					continue;
				}
				auto mark = grid[i][j] == 0 ? &markX : &markO;
				mark->setPosition(getDrawPositionFromGridPosition(i, j));
				window.draw(*mark);
			}
		}

		if (isGameOver)
		{
			window.draw(gameOver);
		}

		window.display();
	}
}
