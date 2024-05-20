def can_build(env, platform):
    return True


def get_opts(platform):
    from SCons.Variables import BoolVariable

    return [
        BoolVariable("graphite", "Enable SIL Graphite smart fonts support", True),
    ]


def configure(env):
    pass


def is_enabled():
    # The module is disabled by default. Use module_text_server_adv_enabled=yes to enable it.
    return False


def get_doc_classes():
    return [
        "TextServerAdvanced",
    ]


def get_doc_path():
    return "doc_classes"
