#include "analyzer.h"

struct ast *all_skills;
struct astpackage *currentpack;
struct astgeneral *currentgeneral;
struct astskill *currentskill;
int indent_level = 0;

void print_indent() {
  for (int i = 0; i < indent_level; i++)
    fprintf(yyout, "  ");
}

void writeline(const char *msg, ...) {
  print_indent();
  va_list ap;
  va_start(ap, msg);

  vfprintf(yyout, msg, ap);
  fprintf(yyout, "\n");
}

static void addgeneralskills(struct ast *skills) {
  struct ast *as = all_skills;
  if (skills->l) {
    addgeneralskills(skills->l);
    while (as->l) {
      struct astskill *s = ((struct astskill *)(as->r));
      if (!strcmp(s->id->str, ((struct aststr *)(skills->r))->str)) {
        writeline("%s:addSkill(%s)", currentgeneral->interid->str, s->interid->str);
        return;
      }
      as = as->l;
    }
    fprintf(stderr, "错误：不存在的技能 \"%s\"\n", ((struct aststr *)(skills->r))->str);
    exit(1);
  }
}

void analyzeGeneral(struct ast *a) {
  checktype(a->nodetype, N_General);

  struct astgeneral *g = (struct astgeneral *)a;
  currentgeneral = g;
  fprintf(yyout, "%s = sgs.General(extension%d, \"%s\", ", g->interid->str, currentpack->uid, g->interid->str);
  analyzeReserved(g->kingdom->str);
  fprintf(yyout, ", %lld)\n", g->hp);
  char buf[64];
  sprintf(buf, "%s", g->interid->str);
  addTranslation(buf, g->id->str);
  sprintf(buf, "#%s", g->interid->str);
  addTranslation(buf, g->nickname->str);
  addgeneralskills(g->skills);
}

void analyzePackageList(struct ast *a) {
  checktype(a->nodetype, N_Packages);

  if (a->l) {
    analyzePackageList(a->l);
  }

  analyzePackage(a->r);
}

void analyzePackage(struct ast *a) {
  checktype(a->nodetype, N_Package);

  struct astpackage *p = (struct astpackage *)a;
  currentpack = p;
  fprintf(yyout, "local extension%d = sgs.Package(\"extension%d\")\n", p->uid, p->uid);
  char buf[64];
  sprintf(buf, "extension%d", p->uid);
  addTranslation(buf, ((struct aststr *)p->id)->str);
  analyzeGeneralList(a->r);
  fprintf(yyout, "\n");
}

void analyzeExtension(struct ast *a) {
  checktype(a->nodetype, N_Extension);

  lookup("你")->type = TPlayer;
  writeline("require \"fkparser\"\n");
  all_skills = a->l;
  analyzeSkillList(a->l);
  analyzePackageList(a->r);
  loadTranslations();
  fprintf(yyout, "\nreturn { ");
  for (int i = 0; i <= currentpack->uid; i++) {
    fprintf(yyout, "extension%d, ", i);
  }
  fprintf(yyout, "}\n");
}

void analyzeGeneralList(struct ast *a) {
  checktype(a->nodetype, N_Generals);

  if (a->l) {
    analyzeGeneralList(a->l);
    analyzeGeneral(a->r);
  }
}
