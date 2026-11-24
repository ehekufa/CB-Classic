package org.cbclassic;

import android.app.Activity;
import android.app.AlertDialog;
import android.os.Handler;
import android.os.Looper;
import android.text.InputType;
import android.util.Log;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.Toast;

/**
 * Настоящие системные окна Android вместо нарисованных движком «карточек».
 *
 * Движок зовёт {@link #ask} когда в игре срабатывает блок «спросить», и
 * {@link #toast} для уведомлений. Ответ диалога уходит обратно в C++ через
 * {@link #nativeResult}: правда — нажата «Принять», null — «Отмена».
 */
public final class NativeUi {
    private static final String TAG = "cbclassic";
    private static Activity activity;
    private static AlertDialog dialog;
    private static final Handler UI = new Handler(Looper.getMainLooper());

    private NativeUi() {
    }

    /** Запомнить активность: диалоги показываются только из неё. */
    public static void attach(Activity current) {
        activity = current;
    }

    public static void detach(Activity current) {
        if (activity == current) {
            dismissDialog();
            activity = null;
        }
    }

    /** Системное всплывающее уведомление (android.widget.Toast). */
    public static void toast(final String text) {
        final Activity current = activity;
        if (current == null || text == null || text.isEmpty()) {
            return;
        }
        UI.post(new Runnable() {
            @Override
            public void run() {
                try {
                    Toast.makeText(current, text, Toast.LENGTH_SHORT).show();
                } catch (RuntimeException e) {
                    Log.w(TAG, "toast не показался", e);
                }
            }
        });
    }

    /**
     * Модальное окно с полем ввода: AlertDialog + EditText (настоящая
     * клавиатура, выделение и удаление текста — всё системное).
     */
    public static void ask(final String title, final String hint, final String initial,
                          final String okText, final String cancelText) {
        final Activity current = activity;
        if (current == null) {
            return;
        }
        UI.post(new Runnable() {
            @Override
            public void run() {
                dismissDialog();
                try {
                    final EditText input = new EditText(current);
                    input.setHint(hint == null ? "" : hint);
                    input.setText(initial == null ? "" : initial);
                    input.setInputType(InputType.TYPE_CLASS_TEXT);
                    input.setSelection(input.getText().length());
                    input.setSingleLine(true);
                    input.setTextSize(20f);

                    AlertDialog.Builder builder = new AlertDialog.Builder(current);
                    if (title != null && !title.isEmpty()) {
                        builder.setTitle(title);
                    }
                    builder.setView(input);
                    builder.setPositiveButton(okText == null ? "OK" : okText,
                            new android.content.DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(final android.content.DialogInterface d, int which) {
                                    dialog = null;
                                    nativeResult(input.getText().toString());
                                }
                            });
                    builder.setNegativeButton(cancelText == null ? "Cancel" : cancelText,
                            new android.content.DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(final android.content.DialogInterface d, int which) {
                                    dialog = null;
                                    nativeResult(null);
                                }
                            });
                    builder.setCancelable(false);
                    dialog = builder.create();
                    dialog.setOnDismissListener(new android.content.DialogInterface.OnDismissListener() {
                        @Override
                        public void onDismiss(final android.content.DialogInterface d) {
                            dialog = null;
                        }
                    });
                    dialog.show();
                    // клавиатура сразу, иначе в полноэкранном режиме её не видно
                    if (dialog.getWindow() != null) {
                        dialog.getWindow().setSoftInputMode(
                                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                        dialog.getWindow().setLayout(
                                (int) (current.getResources().getDisplayMetrics().widthPixels * 0.9),
                                ViewGroup.LayoutParams.WRAP_CONTENT);
                    }
                    input.requestFocus();
                } catch (RuntimeException e) {
                    Log.w(TAG, "диалог не показался", e);
                    nativeResult(null);
                }
            }
        });
    }

    /** Игра закрыла вопрос (смена сцены, выход) — убираем окно. */
    public static void dismiss() {
        UI.post(new Runnable() {
            @Override
            public void run() {
                dismissDialog();
            }
        });
    }

    private static void dismissDialog() {
        if (dialog != null) {
            try {
                dialog.dismiss();
            } catch (RuntimeException e) {
                Log.w(TAG, "диалог не закрылся", e);
            }
            dialog = null;
        }
    }

    /** Ответ из Java в C++ (текст или null). */
    private static native void nativeResult(String text);
}
