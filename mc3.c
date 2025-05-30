#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABU_SIZE 10

// structs
typedef struct
{
  int variable;
  int value;
} TabuEntry;

typedef struct
{
  TabuEntry entries[TABU_SIZE];
  int front;
  int rear;
  int count;
} TabuQueue;

// Function signatures
void initTabuQ(TabuQueue *queue);
int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile);
void readConstraintsMatrix(const char *filename, int constraints[73][73]);
int satisfies(int *Xvalue, int numberofvariables, int numberofvalues, int constraints[73][73]);
int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues, int constraints[73][73]);
int AlternativeAssignment(int *Xvalue, int numberofvariables, int x, int numberofvalues, TabuQueue *TabuList, int *bestConflicts, int constraints[73][73], int *bestCost);
void Tabu_Min_Conflicts(int *Xvalue, int numberofvariables, int numberofvalues, int maxTries, int maxChanges, TabuQueue *TabuList, FILE *outputFile, int *moves, int *bestConflicts, int constraints[73][73]);

// queue initialize
void initTabuQ(TabuQueue *queue)
{
  queue->front = 0;
  queue->rear = -1;
  queue->count = 0;
}

// queue clear
void clearTabuList(TabuQueue *queue)
{
  queue->front = 0;
  queue->rear = -1;
  queue->count = 0;
}

// queue check
int isInTabuList(TabuQueue *queue, int variable, int value)
{
  for (int i = 0; i < queue->count; i++)
  {
    int index = (queue->front + i) % TABU_SIZE;
    if (queue->entries[index].variable == variable && queue->entries[index].value == value)
      return 1;
  }
  return 0;
}

// add to queue
void addToTabuList(TabuQueue *queue, int value, int variable)
{
  if (queue->count == TABU_SIZE)
  {
    queue->front = (queue->front + 1) % TABU_SIZE;
    queue->count--;
  }
  queue->rear = (queue->rear + 1) % TABU_SIZE;
  queue->entries[queue->rear].variable = variable;
  queue->entries[queue->rear].value = value;
  queue->count++;
}

int main()
{
  int maxTries, maxChanges, days, PrecedureRestarts;
  int numberofvariables = 73;

  int *Xvalue = malloc(sizeof(int) * numberofvariables);
  if (!Xvalue)
  {
    fprintf(stderr, "Memory allocation failed.\n");
    return 1;
  }

  printf("Enter the number of tries (random restarts): ");
  scanf("%d", &maxTries);

  if (maxTries < 1)
  {
    printf("Invalid input.\n");
    printf("Enter the number of tries (random restarts): ");
    scanf("%d", &maxTries);
  }

  printf("Enter the number of changes (maxChanges): ");
  scanf("%d", &maxChanges);
  if (maxChanges < 1)
  {
    printf("Invalid input.\n");
    printf("Enter the number of changes (maxChanges): ");
    scanf("%d", &maxChanges);
  }

  printf("Enter the number of days: ");
  scanf("%d", &days);
  if (days < 1)
  {
    printf("Invalid input.\n");
    printf("Enter the number of days: ");
    scanf("%d", &days);
  }
  int numberofvalues = days * 3; // Timeslots = days * 3

  printf("Enter the number of procedure restarts: ");
  scanf("%d", &PrecedureRestarts);
  if (PrecedureRestarts < 1)
  {
    printf("Invalid input.\n");
    printf("Enter the number of procedure restarts: ");
    scanf("%d", &PrecedureRestarts);
  }

  // Open file to save results
  FILE *outputFile = fopen("THIRD.txt", "w");
  if (!outputFile)
  {
    perror("Failed to open THIRD.txt");
    free(Xvalue);
    return 1;
  }

  fprintf(outputFile, "MAX TRIES: %d\n", maxTries);
  fprintf(outputFile, "MAX CHANGES: %d\n", maxChanges);
  fprintf(outputFile, "NUMBER OF DAYS: %d\n", days);
  fprintf(outputFile, "NUMBER OF PROCEDURE RESTARTS: %d\n", PrecedureRestarts);
  fprintf(outputFile, "----------------------------------------------\n");

  int constraints[73][73] = {0};
  readConstraintsMatrix("BetterCSVview.csv", constraints);

  fprintf(outputFile, "RUN RESULTS:\n");
  fprintf(outputFile, "----------------------------------------------\n");

  srand(time(NULL));
  TabuQueue TabuList;
  int totalMoves = 0;
  int totalBestConflicts = 0;
  int solutionsFound = 0;
  double totalExecutionTime = 0.0;

  for (int run = 0; run < PrecedureRestarts; run++)
  {
    initTabuQ(&TabuList);
    int moves = 0, bestConflicts = 0;

    clock_t start = clock();
    Tabu_Min_Conflicts(Xvalue, numberofvariables, numberofvalues, maxTries, maxChanges, &TabuList, outputFile, &moves, &bestConflicts, constraints);
    clock_t end = clock();
    double ExecutionTime = (double)(end - start) / CLOCKS_PER_SEC;

    totalMoves += moves;
    totalBestConflicts += bestConflicts;
    totalExecutionTime += ExecutionTime;
    if (bestConflicts == 0)
      solutionsFound++;

    fprintf(outputFile, "Run %d: Moves = %d, Best Conflicts = %d, Time = %.2f sec\n", run + 1, moves, bestConflicts, ExecutionTime);
  }

  fprintf(outputFile, "\nSUMMARY:\n----------------------------------------------\n");
  fprintf(outputFile, "Solutions Found: %d/%d\n", solutionsFound, PrecedureRestarts);
  fprintf(outputFile, "Average Moves: %.2f\n", (double)totalMoves / PrecedureRestarts);
  fprintf(outputFile, "Average Best Conflicts: %.2f\n", (double)totalBestConflicts / PrecedureRestarts);
  fprintf(outputFile, "Average Execution Time: %.2f sec\n", totalExecutionTime / PrecedureRestarts);

  fclose(outputFile);
  free(Xvalue);
  printf("RESULTS SAVED TO THIRD.txt\n");
  return 0;
}

int *initialize(int *Xvalue, int numberofvariables, int numberofvalues, FILE *outputFile)
{
  // A := initial complete assignment of the variables in Problem
  for (int i = 0; i < numberofvariables; i++)
  {
    Xvalue[i] = rand() % numberofvalues;
  }
  // Print initial assignment
  fprintf(outputFile, "INITIAL ASSIGNMENT:\n");
  for (int i = 0; i < numberofvariables; i++)
  {
    fprintf(outputFile, "X%d = %d\n", i, Xvalue[i]);
  }
  return Xvalue;
}

// Read from CSV file
void readConstraintsMatrix(const char *filename, int constraints[73][73])
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    printf("ERROR OPENING CSV FILE.\n");
    exit(1);
  }

  char buffer[2048];
  int row = 0;

  while (fgets(buffer, sizeof(buffer), file))
  {
    char *token = strtok(buffer, ",");
    int col = 0;

    while (token != NULL)
    {
      // Handle empty cells by assigning 0
      if (strcmp(token, "") == 0 || strcmp(token, "\n") == 0)
      {
        constraints[row][col] = 0;
      }
      else
      {
        constraints[row][col] = atoi(token);
      }
      token = strtok(NULL, ",");
      col++;
    }
    row++;
  }

  fclose(file);
}

// Function to check if constraints are satisfied
int satisfies(int *Xvalue, int numberofvariables, int numberofvalues, int constraints[73][73])
{
  int conflicts = 0;

  // Check constraints...The four types of constraints we have
  for (int i = 0; i < numberofvariables; i++)
  {
    for (int j = i + 1; j < numberofvariables; j++)
    {

      int constraint = constraints[i][j];
      // printf("Checking constraint between X%d and X%d: %d\n", i, j, constraint);

      if (constraint == 1)
      {
        // Xi != Xj
        if (Xvalue[i] == Xvalue[j])
        {
          conflicts++;
        }
      }
      else if (constraint == 2)
      {
        // abs(Xi / 3 - Xj / 3) > 2
        int diff = abs((Xvalue[i] / 3) - (Xvalue[j] / 3));
        if (diff <= 2)
        {
          conflicts++;
        }
      }
      else if (constraint == 3)
      {
        // Xi / 3 != Xj / 3
        if ((Xvalue[i] / 3) == (Xvalue[j] / 3))
        {
          conflicts++;
        }
      }
      else if (constraint == 4)
      {
        // (Xi / 3 == Xj / 3 && Xi % 3 < Xj % 3)
                if ((Xvalue[i] / 3 != Xvalue[j] / 3) || ((Xvalue[i] / 3 == Xvalue[j] / 3) && (Xvalue[i] % 3 >= Xvalue[j] % 3)))
                {
                    // printf("Conflict: X%d / 3 == X%d / 3 && X%d %% 3 >= X%d %% 3\n", i, j, i, j);
                    conflicts++;
                }
      }
    }
  }

  return conflicts; // Total number of conflicts
}

// Function for random variable with conflicts
int RandomVariableConflict(int *Xvalue, int numberofvariables, int numberofvalues, int constraints[73][73])
{

  int list[73], count = 0;
  for (int i = 0; i < numberofvariables; i++)
  {
    for (int j = 0; j < numberofvariables; j++)
    {
      if (i == j)
        continue;
      int conflict = constraints[i][j];
      if ((conflict == 1 && Xvalue[i] == Xvalue[j]) || 
      (conflict == 2 && abs((Xvalue[i] / 3) - (Xvalue[j] / 3)) <= 2) ||
          (conflict == 3 && (Xvalue[i] / 3) == (Xvalue[j] / 3)) || 
          (conflict == 4 && (Xvalue[i] / 3 != Xvalue[j] / 3) || ((Xvalue[i] / 3 != Xvalue[j] / 3) || ((Xvalue[i] / 3 == Xvalue[j] / 3) && (Xvalue[i] % 3 >= Xvalue[j] % 3)))))
      {
        list[count++] = i;
        break;
      }
    }
  }
  return (count == 0) ? rand() % numberofvariables : list[rand() % count];
}

// Function for alternative value
int AlternativeAssignment(int *Xvalue, int numberofvariables, int x, int numberofvalues, TabuQueue *TabuList, int *bestConflicts, int constraints[73][73], int *bestCost)
{
  int bestValue = Xvalue[x];
  int minConflicts = INT_MAX;
  int original = Xvalue[x];

  for (int i = 1; i <= numberofvalues; i++)
  {
    if (i == original)
      continue;
    Xvalue[x] = i;
    int conflict = satisfies(Xvalue, numberofvariables, numberofvalues, constraints);
    if (!isInTabuList(TabuList, x, i) || conflict < *bestConflicts)
    {
      if (conflict < minConflicts)
      {
        minConflicts = conflict;
        bestValue = i;
      }
    }
  }
  Xvalue[x] = original;
  *bestCost = minConflicts; // Store the best conflicts
  return bestValue;
}

// Tabu Search
void Tabu_Min_Conflicts(int *Xvalue, int numberofvariables, int numberofvalues, int maxTries, int maxChanges, TabuQueue *TabuList, FILE *outputFile, int *moves, int *bestConflicts, int constraints[73][73])
{
  *moves = 0;
  *bestConflicts = INT_MAX;
  int *bestAssignment = malloc(sizeof(int) * numberofvariables);

  for (int i = 0; i < maxTries; i++)
  {
    // Initialize the assignment
    // A := initial complete assignment of the variables in Problem
    Xvalue = initialize(Xvalue, numberofvariables, numberofvalues, outputFile);
    clearTabuList(TabuList);

    for (int j = 0; j < maxChanges; j++)
    {
      int conflicts = satisfies(Xvalue, numberofvariables, numberofvalues, constraints);
      if (conflicts == 0)
      {
        *bestConflicts = 0;
        fprintf(outputFile, "Solution found after %d tries and %d changes.\n", i, j);
        fprintf(outputFile, "Total cost: 0\n");
        free(bestAssignment);
        return;
      }

      if (conflicts < *bestConflicts)
      {
        *bestConflicts = conflicts;
        memcpy(bestAssignment, Xvalue, sizeof(int) * numberofvariables);
      }

      int variable = RandomVariableConflict(Xvalue, numberofvariables, numberofvalues, constraints);
      int previous = Xvalue[variable];
      int newVal;
      int bestCost = INT_MAX;
      newVal = AlternativeAssignment(Xvalue, numberofvariables, variable, numberofvalues, TabuList, bestConflicts, constraints, &bestCost);

      Xvalue[variable] = newVal;
      addToTabuList(TabuList, previous, variable);
      (*moves)++;

      fprintf(outputFile, "X%d changed from %d to %d. (Cost : %d) \n", variable, previous, newVal, bestCost);
    }
  }

  fprintf(outputFile, "No solution found. Best total cost: %d\n", *bestConflicts);
  free(bestAssignment);
}
