// 测试带参数的函数定义和调用
int add(int a, int b)
{
    return a + b;
}

void print_result(int result)
{
    // 这是一个void函数
    return;
}

int main()
{
    int x, y, sum;
    x = 5;
    y = 3;
    sum = add(x, y);
    print_result(sum);
    return sum;
}
