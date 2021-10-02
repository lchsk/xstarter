#define mu_assert(message, test) do { \
        if (!(test)) {                  \
            printf("Failure in test: %s\n", __func__); \
            return message;             \
        }                                   \
        } while (0)

#define mu_test(name) \
        char* name()


#define mu_run_test(test) do { char *message = test(); tests_run++; \
                                if (message) return message; } while (0)
extern int tests_run;
