#include <iostream> 
#include <cmath> 
#include <pthread.h> 
#include <thread> 
#include <chrono> 
#include <iomanip> 
#include <vector> 
#include <locale> 

// Функция 6, вариант 
double f(double x)
{
    return log(x) / sqrt(1.2 + 0.3 * x);
}

// Общий расчет суммы трапеций
double trapezoidSum(int start,int end,double a, double h)
{
    double sum = 0.0;

    for (int i = start; i < end; i++)
    {
        double x1 = a + i * h;
        double x2 = a + (i + 1) * h;

        sum += (f(x1) + f(x2))
            / 2.0 * h;
    }

    return sum;
}

// POSIX 
struct ThreadData
{
    int start;
    int end;
    double a;
    double h;
};

void* posixCalculate(void* arg)
{
    ThreadData* data = (ThreadData*)arg;

    double* result = new double;

    *result = trapezoidSum(data->start,data->end,data->a,data->h);

    return result;
}

// Полный расчет POSIX
double calculatePOSIX(int n,int threadCount,double a,double b)
{
    double h = (b - a) / n;

    std::vector<pthread_t> threads(threadCount);
    std::vector<ThreadData> data(threadCount);

    int part = n / threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        data[i].start = i * part;

        if (i == threadCount - 1)
            data[i].end = n;
        else
            data[i].end = (i + 1) * part;

        data[i].a = a;
        data[i].h = h;

        pthread_create(&threads[i], NULL, posixCalculate, &data[i]);
    }

    double result = 0.0;

    for (int i = 0; i < threadCount; i++)
    {
        void* returnedValue = nullptr;

        pthread_join(threads[i],&returnedValue);

        result += *((double*)returnedValue);

        delete (double*)returnedValue;
    }
    return result;
}

// STD::THREAD 
void stdCalculate(int start,int end,double a,double h,double& result)
{
    result = trapezoidSum(start,end,a,h);
}

double calculateSTD(int n,int threadCount,double a,double b)
{
    double h = (b - a) / n;

    std::vector<std::thread> threads(threadCount);
    std::vector<double> results(threadCount);

    int part = n / threadCount;

    for (int i = 0; i < threadCount; i++)
    {
        int start = i * part;

        int end;

        if (i == threadCount - 1)
            end = n;
        else
            end = (i + 1) * part;

        threads[i] = std::thread(stdCalculate,start,end,a,h,std::ref(results[i]));
    }

    double result = 0.0;

    for (int i = 0; i < threadCount; i++)
    {
        threads[i].join();

        result += results[i];
    }

    return result;
}

//тестирование
void testMethod(const std::string& name,
    double (*calc)(int, int, double, double),
    int n, int maxThreads, double a, double b)
{
    std::cout << "\n" << name << "\n";

    double oneThreadTime = 0.0;

    for (int threads = 1; threads <= maxThreads; ++threads)
    {
        auto start = std::chrono::high_resolution_clock::now();
        double result = calc(n, threads, a, b);
        auto finish = std::chrono::high_resolution_clock::now();

        double time = std::chrono::duration<double, std::milli>
            (finish - start).count();

        if (threads == 1) oneThreadTime = time;

        double speedup = oneThreadTime / time;
        double efficiency = speedup / threads * 100.0;

        std::cout << std::fixed << std::setprecision(6)
            << "Потоков: " << threads
            << " | Время: " << time
            << " мс | Результат: " << result
            << " | Ускорение: " << speedup
            << " | Эффективность: " << efficiency << "%\n";
    }
}

// Сравнение одно- и многопоточного вычисления 
void compareResults(int n,int mainThreads,double a,double b)
{
    std::cout << "\n";
    std::cout << "СРАВНЕНИЕ ОДНОПОТОЧНОГО И "
        "МНОГОПОТОЧНОГО ВЫЧИСЛЕНИЯ\n";

    // POSIX THREADS 
    auto startPOSIX1 =
        std::chrono::high_resolution_clock::now();

    double resultPOSIX1 =
        calculatePOSIX(n, 1, a, b);

    auto finishPOSIX1 =
        std::chrono::high_resolution_clock::now();

    double timePOSIX1 =
        std::chrono::duration<double, std::milli>(
            finishPOSIX1 - startPOSIX1
        ).count();

    auto startPOSIXN =
        std::chrono::high_resolution_clock::now();

    double resultPOSIXN =
        calculatePOSIX(n, mainThreads, a, b);

    auto finishPOSIXN =
        std::chrono::high_resolution_clock::now();

    double timePOSIXN =
        std::chrono::duration<double, std::milli>(
            finishPOSIXN - startPOSIXN
        ).count();

    double speedupPOSIX =
        timePOSIX1 / timePOSIXN;

    std::cout << "\nPOSIX\n";
    std::cout << "Однопоточное вычисление:\n";
    std::cout << "Время: "
        << timePOSIX1 << " мс\n";
    std::cout << "Результат: "
        << resultPOSIX1 << "\n";
    std::cout << "\nМногопоточное вычисление:\n";
    std::cout << "Количество потоков: "
        << mainThreads << "\n";
    std::cout << "Время: "
        << timePOSIXN << " мс\n";
    std::cout << "Результат: "
        << resultPOSIXN << "\n";
    std::cout << "Ускорение: "
        << speedupPOSIX << "\n";
    std::cout << "Разница результатов: "
        << std::abs(resultPOSIX1 - resultPOSIXN)
        << "\n";

    // STD::THREAD 
    auto startSTD1 =
        std::chrono::high_resolution_clock::now();

    double resultSTD1 =
        calculateSTD(n, 1, a, b);

    auto finishSTD1 =
        std::chrono::high_resolution_clock::now();

    double timeSTD1 =
        std::chrono::duration<double, std::milli>(
            finishSTD1 - startSTD1
        ).count();

    auto startSTDN =
        std::chrono::high_resolution_clock::now();

    double resultSTDN =
        calculateSTD(n, mainThreads, a, b);

    auto finishSTDN =
        std::chrono::high_resolution_clock::now();

    double timeSTDN =
        std::chrono::duration<double, std::milli>(
            finishSTDN - startSTDN
        ).count();

    double speedupSTD =
        timeSTD1 / timeSTDN;

    std::cout << "\nstd::thread\n";
    std::cout << "Однопоточное вычисление:\n";
    std::cout << "Время: "
        << timeSTD1 << " мс\n";
    std::cout << "Результат: "
        << resultSTD1 << "\n";
    std::cout << "\nМногопоточное вычисление:\n";
    std::cout << "Количество потоков: "
        << mainThreads << "\n";
    std::cout << "Время: "
        << timeSTDN << " мс\n";
    std::cout << "Результат: "
        << resultSTDN << "\n";
    std::cout << "Ускорение: "
        << speedupSTD << "\n";
    std::cout << "Разница результатов: "
        << std::abs(resultSTD1 - resultSTDN)
        << "\n";
}

int main()
{
    setlocale(LC_ALL, "Russian");

    int n;
    int mainThreads;
    int maxThreads;

    double a = 5.0;
    double b = 10.0;

    std::cout
        << "Функция: ln(x) / sqrt(1.2 + 0.3*x)\n";
    std::cout
        << "Отрезок: [" << a << "; " << b << "]\n\n";
    std::cout
        << "Введите количество разбиений: ";
    std::cin >> n;
    std::cout
        << "Введите количество потоков "
        "для основного вычисления: ";
    std::cin >> mainThreads;
    std::cout
        << "Введите максимальное количество "
        "потоков для тестирования: ";
    std::cin >> maxThreads;

    // Проверки 
    if (n <= 0 ||
        mainThreads <= 0 ||
        maxThreads <= 0)
    {
        std::cout
            << "\nОшибка: все значения "
            "должны быть больше нуля.\n";

        return 1;
    }
    if (mainThreads > maxThreads)
    {
        std::cout
            << "\nОшибка: количество потоков "
            "основного вычисления не должно "
            "превышать максимальное количество "
            "потоков.\n";

        return 1;
    }
    if (maxThreads > n)
    {
        std::cout
            << "\nОшибка: количество потоков "
            "не должно превышать количество "
            "разбиений.\n";

        return 1;
    }
    testMethod("POSIX THREADS", calculatePOSIX, n, maxThreads, a, b);
    testMethod("STD::THREAD", calculateSTD, n, maxThreads, a, b);

    compareResults(n, mainThreads, a, b);

    return 0;
}