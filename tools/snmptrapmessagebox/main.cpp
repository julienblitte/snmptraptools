#include <windows.h>
#include <stdio.h>

#define ARG_AGENT     1
#define ARG_OID       2
#define ARG_GENERIC   3
#define ARG_SPECIFIC  4

char buffer[1024];

const char *trapname[] = { "cold start", "warm start", "link down", "link up",
                        "authentication failure", "routing table updated", "enterprise specific alert" };

const char *trapdesc[] =
{
    "Agent %s has started.",
    "Agent %s has restared, maybe due to a configuration change.",
    "Network interface of agent %s have been change its state to down.",
    "Network interface of agent %s have been change its state to up.",
    "Unauthorised person attempted to access to the agent %s.",
    "The routing table of agent %s has been updated.",
    "Specific event of manufacturer of agent %s."
};

const char *get_trap_eventname(unsigned int generic_type)
{
    if (generic_type >= 0 && generic_type <= 6)
    {
        return trapname[generic_type];
    }
    else
    {
        return "unknow";
    }
}

unsigned int get_trap_code(unsigned int generic_type, unsigned int specific_type)
{
    return (generic_type == 6 ? specific_type : generic_type);
}

const char *get_trap_eventdescription(unsigned int generic_type, char *agent)
{
    static char description_buffer[256];
    if (generic_type >= 0 && generic_type <= 6)
    {
        snprintf(description_buffer, sizeof(description_buffer), trapdesc[generic_type], agent);
        return description_buffer;
    }
    else
    {
        return "";
    }
}

int main(int argc, char *argv[])
{
    unsigned int generic_type;
    unsigned int specific_type;

    //snprintf(param, sizeof(param), "%s %s %ld %ld", trap->agent, trap->enterprise, trap->genericTrap, trap->specificTrap);
    if (argc != 5)
    {
        MessageBoxA(NULL, "Bad arguments.\nArguments should be: {agent, oid, generic type, specific type}", "snmptraptools", MB_OK|MB_ICONERROR);
        return EXIT_FAILURE;
    }
    sscanf(argv[ARG_GENERIC], "%d", &generic_type);
    sscanf(argv[ARG_SPECIFIC], "%d", &specific_type);

    snprintf(buffer, sizeof(buffer),
             "Agent: %s\n\nOID:%s\nTrap code is %d\n\n%s",
             argv[ARG_AGENT], // agent
             argv[ARG_OID], // oid
             get_trap_code(generic_type, specific_type),
             get_trap_eventdescription(generic_type, argv[ARG_AGENT])
             );
    MessageBoxA(NULL, buffer, get_trap_eventname(generic_type), MB_OK|MB_ICONINFORMATION|MB_SERVICE_NOTIFICATION);

    LocalFree(argv);

    return EXIT_SUCCESS;
}
