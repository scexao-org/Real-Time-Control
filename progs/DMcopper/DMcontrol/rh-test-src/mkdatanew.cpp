#include <fstream>
#include <iostream>

using namespace std;

int main(void)
{
	int i = 0;
	int voltage = -1;
	int buffer_value = 0;

    while ((voltage < 0) or (voltage > 300))
    {
        cout << "\nInput piston voltage: ";
        cin >> voltage;

        if ((voltage < 0) or (voltage > 300))
        {
            cout << "\nInput voltage " << voltage << " is not in the range 0 - 300\n";
        }
		else
		{
			cout << "\n";
		}
    }

	buffer_value = (voltage * 65535)/300;

	ofstream fout("data1024.txt");

	for (i = 1; i <= 1024; i++)
	{
		fout << uppercase << hex << buffer_value << "\n";
	}

	ofstream foutl("data4096.txt");

    for (i = 1; i <= 1024; i++)
    {
        foutl << uppercase << hex << buffer_value << "\n";
    }
	for (i = 1025; i <= 4096; i++)
	{
		foutl << 0 << "\n";
	}

	fout.close();
	foutl.close();

    	return 0;
}
