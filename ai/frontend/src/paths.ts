import { ApiError } from "./api";
import type { PathStatus } from "./desktop";
import { t } from "./i18n";

export function projectPathProblem(status: PathStatus): string {
  if (status === "missing") {
    return t(
      "پوشه‌ی این پروژه روی این رایانه وجود ندارد. ویرایش پروژه را باز کنید، یک پوشه‌ی موجود انتخاب کنید و دوباره اجرا کنید.",
      "The folder of this project does not exist on this computer. Open project edit, choose an existing folder, and run the chat again.",
    );
  }
  return t(
    "مسیر این پروژه پوشه‌ی معتبری نیست. ویرایش پروژه را باز کنید و یک پوشه‌ی موجود انتخاب کنید.",
    "The path of this project is not a valid folder. Open project edit and choose an existing folder.",
  );
}

export function projectPathUnset(): string {
  return t(
    "برای این پروژه هنوز پوشه‌ای انتخاب نشده است. ویرایش پروژه را باز کنید و یک پوشه‌ی موجود انتخاب کنید.",
    "This project has no folder yet. Open project edit and choose an existing folder.",
  );
}

export function extraPathProblem(): string {
  return t(
    "پوشه‌ی اضافه‌ی این گفت‌وگو روی این رایانه وجود ندارد. آن را عوض کنید یا بردارید.",
    "The extra folder of this chat is not an existing folder on this computer. Change or remove it.",
  );
}

export function pathErrorText(e: unknown): string | null {
  if (!(e instanceof ApiError)) return null;
  switch (e.code) {
    case "project_path":
      return projectPathUnset();
    case "path_locked":
      return t(
        "تا وقتی گفت‌وگویی در این پروژه در حال اجراست، پوشه‌ی پروژه را نمی‌شود عوض کرد. صبر کنید تا تمام شود یا آن را متوقف کنید.",
        "The project folder cannot be changed while a chat in this project is running. Wait for it to finish or stop it.",
      );
    case "path":
      return t(
        "پوشه‌ی پروژه باید یک مسیر کامل و معتبر باشد.",
        "The project folder must be a full, valid path.",
      );
    case "extra_path":
      return t(
        "پوشه‌ی اضافه باید یک مسیر کامل و معتبر باشد.",
        "The extra folder must be a full, valid path.",
      );
    default:
      return null;
  }
}
