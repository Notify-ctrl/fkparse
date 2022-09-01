vim9script

# A script that make add new built-in func/var much more faster
# only should be used under fkparse's git repo in Vim

# parse a function def, then add it to analyze_qsan/builtin/action.c
# def format just like fkp's funcdef format
# e.g. __addMark(玩家:TPlayer,标记名:TString,数量:TNumber=1)
def NewFunc(funcdef: string)
  var func_end = match(funcdef, "(")
  var func_name = strpart(funcdef, 0, func_end)
  var short_func_name = strpart(funcdef, 2, func_end - 2)
  var params = split(strpart(funcdef, func_end + 1, strlen(funcdef) - func_end - 2), ",")
  var lua_func = "fkp.functions." .. short_func_name

  # go to the root dir, then open files to edit
  var root_dir = system("git rev-parse --show-toplevel")
  var curr_dir = getcwd()
  exec "lcd " .. root_dir

  # First, add funcdef to action.c
  # maybe need to move it
  exec "e src/analyze_qsan/builtin/action.c"
  exec ":/{NULL, NULL, TNone, 0, {}}"
  var line1 = "{\"" .. func_name .. "\", \"" .. lua_func .. "\", TNone, " .. len(params) .. ", {"
  exec "normal O" .. line1 .. "\<CR>\<Esc>"

  var i = 0
  while i < len(params)
    var param = substitute(params[i], "=", ":", "g")
    var par_list = split(param, ":")
    var line = "{\"" .. par_list[0] .. "\", " .. par_list[1] .. ", false, {.s = NULL}},"
    exec "normal i    " .. line .. "\<CR>\<Esc>"
    i = i + 1
  endwhile
  exec "normal i  }},\<Esc>"
  exec "w"

  # now let's go to fkparser.lua and add funcdef here
  # (need correct it manually)
  exec "e lua/fkparser.lua"
  exec ":/function fkp.newlist"
  line1 = lua_func .. " = function("
  exec "normal O" .. line1 .. "\<Esc>"

  i = 0
  while i < len(params)
    var param = substitute(params[i], "=", ":", "g")
    var par_list = split(param, ":")
    exec "normal A" .. par_list[0] .. "\<Esc>"
    i = i + 1
    if i < len(params)
      exec "normal A, \<Esc>"
    else
      exec "normal A)\<CR>\<CR>end\<CR>\<Esc>"
    endif
  endwhile
  exec "w"

  # Finally, go back to origin work dir
  exec "lcd " .. curr_dir
enddef

command -nargs=+ FKPnewfunc :call NewFunc(<q-args>)

