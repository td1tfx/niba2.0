import yaml
import subprocess
import sys

if len(sys.argv) < 4:
    print("gen_structs.py structs.yaml structs.h structs.cpp")
    exit(-1)

with open(sys.argv[1], 'r') as stream:
    structs = yaml.load(stream)


class Field:
    def __init__(self, ctype, name, default):
        self.type = ctype
        self.name = name
        self.default = default

    def get_def(self):
        if self.default is None:
            return "{} {};".format(self.type, self.name)
        else:
            return "{} {}{{{}}};".format(self.type, self.name, self.default)

    @staticmethod
    def from_dict(field):
        return Field(field.get('type', 'int'), field['name'], field.get('default'))


lh = []
lx = []

lh.append('// this file is generated, see structs.yaml and gen_structs.py')
lh.append('')
lh.append('#pragma once')
lh.append('')
lh.append('#include "global_defs.h"')
lh.append('#include <nlohmann/json.hpp>')
lh.append('')
lh.append('namespace nibashared {')
lh.append('')

lx.append('// this file is generated, see structs.yaml and gen_structs.py')
lx.append('')
lx.append('#include "structs.h"')
lx.append('')
lx.append('namespace nibashared {')
lx.append('')

for tdef in structs['typedef']:
    lh.append('using {} = {};'.format(tdef['name'], tdef['type']))
lh.append('')
for struct in structs['structs']:
    struct_name = struct['name']
    lh.append('struct {} {{'.format(struct_name))
    fields = []
    for field in struct['fields']:
        if 'name' in field:
            fields.append(Field.from_dict(field))
        elif 'merge' in field:  # WIP
            # TODO make this recursive, and do other cool stuff with it
            for sub_fields in field['merge']:
                fields.append(Field.from_dict(sub_fields))
    for field in fields:
        lh.append(field.get_def())
    lh.append('')
    if struct.get('addition'):
        lh.append('{struct_name} &operator+=(const {struct_name} &rhs) {{'.format(
                struct_name=struct_name))
        for field in fields:
            lh.append('{field_name} += rhs.{field_name};'.format(field_name=field.name))
        lh.append('return *this;')
        lh.append('}')
        lh.append('')
    lh.append('};')
    to_json = 'void to_json(nlohmann::json &j, const {struct_name} &{struct_name})'.format(
            struct_name=struct_name)
    
    from_json = 'void from_json(const nlohmann::json &j, {struct_name} &{struct_name})'.format(
            struct_name=struct_name)
    
    lh.append(to_json + ';')
    lh.append(from_json + ';')
    lh.append('')
    
    lx.append('')
    lx.append(to_json + '{')
    lx.append('j = nlohmann::json{')
    for field in fields:
        lx.append('{{ "{field_name}", {struct_name}.{field_name} }},'.format(
                struct_name=struct_name, field_name=field.name))
    lx.append('};')
    lx.append('}')
    
    lx.append('')
    lx.append(from_json + '{')
    for field in fields:
        lx.append('j.at("{field_name}").get_to({struct_name}.{field_name});'.format(
                field_name=field.name, struct_name=struct_name))
    lx.append('}')
    lx.append('')
lh.append('}')
lx.append('}')

header = sys.argv[2]
with open(header, 'w', encoding='utf8') as file:
    file.write('\n'.join(lh))

cxx = sys.argv[3]
with open(cxx, 'w', encoding='utf8') as file:
    file.write('\n'.join(lx))

subprocess.run(['clang-format', '-style', 'file', '-i', header])
subprocess.run(['clang-format', '-style', 'file', '-i', cxx])