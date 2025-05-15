int main()
{
    int a, c;
    int b;
    int d;
    a = 010;
    b = 0xf;
    d = -5;
    {
        int c;
        c = a * b;
        b = c / a;
        a = a % b;

        return a + b + c;
    }
}