#include "cli.h"
#include "new.h"

cliparser* create_cli_parser()
{
    cliparser *clip = cliparser_new("cocadoc", "COCADA source code documentation");
    return clip;
}

int main(int argc, char** argv)
{
    cliparser *clip = create_cli_parser();
    cliparse_res res = cliparser_parse(clip, argc, argv, true);

    DESTROY_FLAT(clip, cliparser);

}
