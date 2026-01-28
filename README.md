# Build & Run

## Как собрать и запустить

### Lab1
- Выполнить `./run_lab1.sh` (команда собирает `ast_dump`, парсит все `Lab1/examples/*.txt` и сохраняет DOT/PDF в `Lab1/examples/`).
  Скрипт выводит результаты и подготовленные артефакты.

### Lab2
- Перейти в `Lab2/` и запустить `./run_tests.sh`, он компилирует `lab2_cfg`, вызывает `Lab2/generate_cfgs.sh`, генерирует CFG и call-graph в `Lab2/out/`, а также создает `out.asm`.
  Пример: `./Lab2/run_tests.sh`.

### Lab3
- Выполнить `./run_lab3.sh`. Скрипт делает `chmod +x build/build_script.sh build/execute_script.sh`, переходит в `build/`, передает `LOGIN/PASSWORD` через `BUILD_LOGIN`/`BUILD_PASSWORD`, вызывает `build_script.sh` (сборка) и `execute_script.sh` (запуск).
  Результат — готовый `out.asm` и отчеты из `build/out/`.

## Структура

- `Lab1/examples/` — исходные файлы и соответствующие DOT/PDF.
- `Lab2/out/` — CFG, call-graph, DOT/PDF и `out.asm`.
- `reports/` — Markdown-отчёты (Lab1, Lab2, Lab3).
- `architecture/` содержит описание VM и примеры `*.asm`.
- `build/` исполняет удаленную сборку/запуск; `run_lab3.sh` использует эти скрипты и передает логин/пароль через переменные, которые можно переопределить.

## Отчёты

Файлы `reports/lab1.md`, `reports/lab2.md`, `reports/lab3.md` содержат готовый текст отчётов (Цели, Задачи, Описание, Аспекты реализации, Результаты, Тесты, Выводы). Их можно открыть вручную и при необходимости конвертировать в PDF на своей машине.

## Важно

- `run_lab3.sh` передаёт `BUILD_LOGIN`/`BUILD_PASSWORD` в `build/build_script.sh` и `build/execute_script.sh`; при необходимости можно задать другие учетные данные через переменные окружения.
- Скрипты ожидают, что `build/` существует и содержит `build_script.sh`/`execute_script.sh`, которые уже идут с проектом.

## Примечания

- Видео находится по ссылке: https://disk.yandex.ru/d/lMH1GMnbU9334Q
- `out.asm` — результирующий линейный код для Lab3.
- `Lab2/out/` и `Lab1/examples/` — готовые артефакты, проверенные генераторами CFG и linear.
