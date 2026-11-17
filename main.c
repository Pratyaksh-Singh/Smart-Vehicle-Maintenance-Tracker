#include <stdio.h>
#include <stdlib.h>

#define SERVICE_KM    10000
#define SERVICE_DAYS  180
#define REMIND_DAYS   15

struct Vehicle {
    char number[20];
    int day, month, year;
    long serviceKm;
    long currentKm;
};

const int cumDays[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

int validDate(int d, int m, int y)
{
    return d >= 1 && d <= 31 && m >= 1 && m <= 12 && y >= 2000;
}

int toDays(int d, int m, int y)
{
    int days = y * 365 + y / 4 - y / 100 + y / 400 + cumDays[m - 1] + d;
    int leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;

    if (leap && m <= 2)
        days--;
    return days;
}

double averageDaily(struct Vehicle *v, int today)
{
    int passed = today - toDays(v->day, v->month, v->year);

    if (passed <= 0)
        return 0;
    return (double)(v->currentKm - v->serviceKm) / passed;
}

int daysLeft(struct Vehicle *v, int today)
{
    int passed = today - toDays(v->day, v->month, v->year);
    long kmLeft = SERVICE_KM - (v->currentKm - v->serviceKm);
    double avg = averageDaily(v, today);
    int byTime = SERVICE_DAYS - passed;
    int byKm = 9999;

    if (kmLeft < 0)
        return -1;
    if (avg > 0)
        byKm = kmLeft / avg;

    if (byTime < byKm)
        return byTime;
    return byKm;
}

const char *status(int left)
{
    if (left < 0)
        return "OVERDUE";
    if (left <= REMIND_DAYS)
        return "DUE SOON";
    return "OK";
}

struct Vehicle *addVehicle(struct Vehicle *list, int *count, int today)
{
    struct Vehicle v = {0};
    struct Vehicle *bigger;
    int got = 0;

    printf("Vehicle number: ");
    got += scanf("%19s", v.number);
    printf("Last service date (DD MM YYYY): ");
    got += scanf("%d %d %d", &v.day, &v.month, &v.year);
    printf("Odometer at last service (km): ");
    got += scanf("%ld", &v.serviceKm);
    printf("Current odometer (km): ");
    got += scanf("%ld", &v.currentKm);

    if (got != 6 || !validDate(v.day, v.month, v.year) ||
        toDays(v.day, v.month, v.year) > today ||
        v.currentKm < v.serviceKm) {
        printf("Invalid details - vehicle not added.\n");
        return list;
    }

    bigger = realloc(list, (*count + 1) * sizeof(struct Vehicle));
    if (bigger == NULL) {
        printf("Out of memory - vehicle not added.\n");
        return list;
    }
    list = bigger;
    list[*count] = v;
    (*count)++;
    printf("Vehicle added.\n");
    return list;
}

void showAll(struct Vehicle *list, int count, int today)
{
    int i, left;

    if (count == 0) {
        printf("No vehicles added yet.\n");
        return;
    }
    printf("\n%-12s %-12s %-9s %-10s %-10s %s\n", "Vehicle", "Last Service",
           "Odometer", "Avg km/day", "Days Left", "Status");
    for (i = 0; i < count; i++) {
        left = daysLeft(&list[i], today);
        printf("%-12s %02d-%02d-%04d   %-9ld %-10.1f %-10d %s\n", list[i].number,
               list[i].day, list[i].month, list[i].year, list[i].currentKm,
               averageDaily(&list[i], today), left, status(left));
    }
}

void showAlerts(struct Vehicle *list, int count, int today)
{
    int i, left, found = 0;

    printf("\n--- SERVICE ALERTS ---\n");
    for (i = 0; i < count; i++) {
        left = daysLeft(&list[i], today);
        if (left < 0) {
            printf("[OVERDUE]  %s - service is overdue! Send it to the workshop now.\n",
                   list[i].number);
            found++;
        } else if (left <= REMIND_DAYS) {
            printf("[REMINDER] %s - service due in %d day(s).\n", list[i].number, left);
            found++;
        }
    }
    if (found == 0)
        printf("No alerts. All vehicles are fine.\n");
}

int main(void)
{
    struct Vehicle *list = NULL;
    int count = 0, choice, d = 0, m = 0, y = 0, today;

    printf("Enter today's date (DD MM YYYY): ");
    if (scanf("%d %d %d", &d, &m, &y) != 3 || !validDate(d, m, y)) {
        printf("Invalid date.\n");
        return 1;
    }
    today = toDays(d, m, y);

    do {
        printf("\n1. Add vehicle\n2. Show all vehicles\n3. Show alerts\n0. Exit\nChoice: ");
        if (scanf("%d", &choice) != 1)
            break;
        if (choice == 1)
            list = addVehicle(list, &count, today);
        else if (choice == 2)
            showAll(list, count, today);
        else if (choice == 3)
            showAlerts(list, count, today);
    } while (choice != 0);

    free(list);
    return 0;
}
