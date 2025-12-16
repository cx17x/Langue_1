Примеры и команды для запуска парсера (вариант 2)

Генерация парсера (в папке `Lab1`):

  npx tree-sitter generate

Сборка утилиты `ast_dump` из корня проекта:

  clang ast_dump.c Lab1/src/parser.c vendor/tree-sitter/lib/src/lib.c \
    -I vendor/tree-sitter/lib/include -I vendor/tree-sitter/lib/src -o ast_dump

Запуск на примере и генерация `.dot`:

  ./ast_dump Lab1/examples/func_with_params.txt Lab1/examples/func_with_params.dot

Преобразование в изображение (если установлен `dot`):

  dot -Tpng Lab1/examples/func_with_params.dot -o Lab1/examples/func_with_params.png

Список примеров находится в `Lab1/examples/`.
Автоматически парсить все примеры и генерировать PDF-деревья:

  # сделать исполняемым (один раз)
  chmod +x Lab1/generate_trees.sh

  # из корня проекта
  Lab1/generate_trees.sh

Скрипт построит `ast_dump` (если он отсутствует), распарсит все `*.txt` в
`Lab1/examples/` и создаст `.dot` и `.pdf` для каждого примера (при наличии
Graphviz `dot`).

**Какие цвета вообще есть (цвета в PDF-дереве)**

- `assignment` — светло-голубой прямоугольник (оператор присваивания)
- `if_statement` — светло-зелёный прямоугольник (условный оператор)
- `funcDef` — серый эллипс (объявление/определение функции/метода)
- `varDecl` — светло-жёлтый прямоугольник (объявление переменных)

Эти подсказки помогают быстро найти ключевые конструкции в дереве.
