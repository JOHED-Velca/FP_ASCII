#include <iostream>
#include <chrono>
using namespace std;

#include <Windows.h> //to be able to get hold of the console

int nScreenWidth = 120; //columns
int nScreenHeight = 40; //rows

//Store where the player is
float fPlayerX = 8.0f;
float fPlayerY = 8.0f;

float fPlayerA = 0.0f; //the angle the player is looking at

//Map
int nMapHeight = 16;
int nMapWidth = 16;

float fFOV = 3.14159 / 4.0; //field of view

float fDepth = 16.0f; 

int main() {
   // Create Screen Buffer
   wchar_t* screen = new wchar_t[nScreenWidth * nScreenHeight];
   HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
   SetConsoleActiveScreenBuffer(hConsole);
   DWORD dwBytesWritten = 0;

   //Create the map
   wstring map;

   map += L"################";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..........#...#";
   map += L"#..........#...#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#..............#";
   map += L"#.......########";
   map += L"#..............#";
   map += L"#..............#";
   map += L"################";

   //we need two time point to measure the duration
   auto tp1 = chrono::system_clock::now();
   auto tp2 = chrono::system_clock::now();


   //Game loop
   while (1)
   {
      //For each iteration of the game loop
      tp2 = chrono::system_clock::now(); //grub the current system time
      chrono::duration<float> elapsedTime = tp2 - tp1; //calculate the duration between the current system time and the previous
      tp1 = tp2; //update the old time point
      float fElapsedTime = elapsedTime.count(); //put the elapsed time in float because all the game is in float

      //CONTROLS
      //Handle CCW Rotation
      
      //counter clockwise decrease the angle
      if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
         fPlayerA -= (0.8f) * fElapsedTime;

      //clockwise encrease the angle
      if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
         fPlayerA += (0.8f) * fElapsedTime;

      //When the character is walking forward
      if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
      {
         fPlayerX += sinf(fPlayerA) * 5.0f * fElapsedTime;
         fPlayerY += cosf(fPlayerA) * 5.0f * fElapsedTime;
      }

      //When the character is walking backwards
      if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
      {
         fPlayerX -= sinf(fPlayerA) * 5.0f * fElapsedTime; //takes the unit vector that we calculated for the ray, multiplies to give it a magnitud
         fPlayerY -= cosf(fPlayerA) * 5.0f * fElapsedTime;
      }

      for (int x = 0; x < nScreenWidth; x++)
      {
         //For each column, calculate the project ray angle into world space
         float fRayAngle = (fPlayerA - fFOV / 2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

         float fDistanceToWall = 0; //distance from the player to the wall for that given angle
         bool bHitWall = false;

         float fEyeX = sinf(fRayAngle); //unit vector for ray in player space
         float fEyeY = cosf(fRayAngle);

         while (!bHitWall && fDistanceToWall < fDepth)
         {
            fDistanceToWall += 0.1f;

            //Create a line of a given distance
            int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
            int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

            //Test if ray is out of bounds
            if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight) //if you are outside of bounds
            {
               bHitWall = true; //Just set distance to maximum depth
               fDistanceToWall = fDepth;
            }
            else //if you are within bounds
            {
               //Ray is inbounds so test to see if the ray cell is a wall block
               if (map[nTestY * nMapWidth + nTestX] == '#') //Converting the 2D system into a 1D for the array
               {
                  bHitWall = true;
               }
            }
         }

         //Calculate distance to the ceiling and floor
         int nCeiling = (float)(nScreenHeight / 2.0) - nScreenHeight / ((float)fDistanceToWall);
         int nFloor = nScreenHeight - nCeiling;

         short nShade = ' ';

         if (fDistanceToWall <= fDepth / 4.0f)           nShade = 0x2588;  //Very Close
         else if (fDistanceToWall < fDepth / 3.0f)       nShade = 0x2593;
         else if (fDistanceToWall < fDepth / 2.0f)       nShade = 0x2592;
         else if (fDistanceToWall < fDepth)              nShade = 0x2591;
         else                                            nShade = ' ';     //Too far away

         //draw into the column
         for (int y = 0; y < nScreenHeight; y++)
         {
            if (y < nCeiling)
               screen[y * nScreenWidth + x] = ' ';
            else if(y > nCeiling && y <= nFloor)
               screen[y * nScreenWidth + x] = nShade;
            else
               screen[y * nScreenWidth + x] = ' ';
         }

      }
      //To write on the screen
      screen[nScreenWidth * nScreenHeight - 1] = '\0';
      WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);
   }

   return 0;
}