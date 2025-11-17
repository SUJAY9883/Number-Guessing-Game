#include <gtk/gtk.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int random_num;
int number_of_guesses;

GtkWidget *window;
GtkWidget *feedback_label;
GtkWidget *guess_entry;
GtkWidget *name_entry;
GtkWidget *start_button;
GtkWidget *guess_button;

// ---------------- RESPONSIVE CSS ----------------
void apply_responsive_styles(GtkWidget *widget) {
    GdkDisplay *display = gdk_display_get_default();
    GdkMonitor *monitor = gdk_display_get_primary_monitor(display);

    GdkRectangle geo;
    gdk_monitor_get_geometry(monitor, &geo);
    int width = geo.width;

    int font_size = (width <= 600) ? 16 :
                    (width <= 1200) ? 22 : 26;

    GtkCssProvider *provider = gtk_css_provider_new();

    char css[700];
    snprintf(css, sizeof(css),
        "* { font-size: %dpx; } "
        ".main-button {"
            "background: #4a90e2;"
            "color: white;"
            "padding: 14px;"
            "border-radius: 12px;"
            "font-weight: bold;"
        "}"
        ".entry-style {"
            "padding: 10px;"
            "border-radius: 8px;"
        "}"
        "window { background: #eef2f7; }",
        font_size
    );

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );
}

// ---------------- GAME LOGIC ----------------
void start_new_game(GtkWidget *widget, gpointer data) {
    const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
    if (strlen(name) == 0) {
        gtk_label_set_text(GTK_LABEL(feedback_label),
            "❗ Please enter your name first.");
        return;
    }

    srand(time(0));
    random_num = (rand() % 100) + 1;
    number_of_guesses = 0;

    char msg[256];
    snprintf(msg, sizeof(msg),
        "Hello %s! I picked a number (1–100).\nTry to guess it!",
        name);

    gtk_label_set_text(GTK_LABEL(feedback_label), msg);

    gtk_widget_set_sensitive(guess_entry, TRUE);
    gtk_widget_set_sensitive(guess_button, TRUE);
    gtk_widget_set_sensitive(name_entry, FALSE);
    gtk_widget_set_sensitive(start_button, FALSE);

    gtk_entry_set_text(GTK_ENTRY(guess_entry), "");
}

void check_guess(GtkWidget *widget, gpointer data) {
    const char *guess_text = gtk_entry_get_text(GTK_ENTRY(guess_entry));
    int guessed_num;
    char msg[256];

    if (sscanf(guess_text, "%d", &guessed_num) != 1) {
        gtk_label_set_text(GTK_LABEL(feedback_label),
            "❗ Invalid input. Enter a number between 1–100.");
        return;
    }

    if (guessed_num < 1 || guessed_num > 100) {
        snprintf(msg, sizeof(msg),
            "❗ %d is invalid! Enter a number between 1–100.", guessed_num);
        gtk_label_set_text(GTK_LABEL(feedback_label), msg);
        return;
    }

    number_of_guesses++;

    if (guessed_num < random_num) {
        snprintf(msg, sizeof(msg),
            "Guess %d: %d is SMALLER than the number.\nTry a higher number!",
            number_of_guesses, guessed_num);
        gtk_label_set_text(GTK_LABEL(feedback_label), msg);
    }
    else if (guessed_num > random_num) {
        snprintf(msg, sizeof(msg),
            "Guess %d: %d is GREATER than the number.\nTry a lower number!",
            number_of_guesses, guessed_num);
        gtk_label_set_text(GTK_LABEL(feedback_label), msg);
    }
    else {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const char *performance;

        if (number_of_guesses <= 3) performance = "Outstanding!";
        else if (number_of_guesses <= 5) performance = "Excellent!";
        else if (number_of_guesses <= 7) performance = "Good!";
        else if (number_of_guesses <= 10) performance = "Average!";
        else performance = "Needs improvement!";

        snprintf(msg, sizeof(msg),
            "🎉 Congratulations %s!\n"
            "Correct Number: %d\n"
            "Total Guesses: %d\n"
            "Performance: %s",
            name, random_num, number_of_guesses, performance);

        gtk_label_set_text(GTK_LABEL(feedback_label), msg);

        gtk_widget_set_sensitive(guess_entry, FALSE);
        gtk_widget_set_sensitive(guess_button, FALSE);
        gtk_widget_set_sensitive(name_entry, TRUE);
        gtk_widget_set_sensitive(start_button, TRUE);
        gtk_button_set_label(GTK_BUTTON(start_button), "Play Again?");
    }

    gtk_entry_set_text(GTK_ENTRY(guess_entry), "");
}

// ---------------- GUI SETUP ----------------
static void activate(GtkApplication *app, gpointer user_data) {

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Number Guessing Game");

    // ✔ REAL WINDOWS TITLEBAR: Minimize, Maximize, Close
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 800);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);

    apply_responsive_styles(window);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 20);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 40);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title),
        "<span weight='bold' size='30000'>Number Guessing Game</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);

    // --- Name Section ---
    GtkWidget *name_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), name_box, FALSE, FALSE, 0);

    GtkWidget *name_label = gtk_label_new("Your Name:");
    gtk_box_pack_start(GTK_BOX(name_box), name_label, FALSE, FALSE, 0);

    name_entry = gtk_entry_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(name_entry), "entry-style");
    gtk_box_pack_start(GTK_BOX(name_box), name_entry, TRUE, TRUE, 0);

    start_button = gtk_button_new_with_label("Start Game");
    gtk_style_context_add_class(gtk_widget_get_style_context(start_button), "main-button");
    g_signal_connect(start_button, "clicked", G_CALLBACK(start_new_game), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), start_button, FALSE, FALSE, 0);

    // --- Feedback Label ---
    feedback_label = gtk_label_new("Enter your name and press Start.");
    gtk_label_set_line_wrap(GTK_LABEL(feedback_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(feedback_label), 0.5);
    gtk_label_set_yalign(GTK_LABEL(feedback_label), 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), feedback_label, FALSE, FALSE, 10);

    // --- Guess Section ---
    GtkWidget *guess_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(vbox), guess_box, FALSE, FALSE, 0);

    GtkWidget *guess_label = gtk_label_new("Your Guess:");
    gtk_box_pack_start(GTK_BOX(guess_box), guess_label, FALSE, FALSE, 0);

    guess_entry = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(guess_entry), 3);
    gtk_style_context_add_class(gtk_widget_get_style_context(guess_entry), "entry-style");
    gtk_box_pack_start(GTK_BOX(guess_box), guess_entry, TRUE, TRUE, 0);

    guess_button = gtk_button_new_with_label("Guess");
    gtk_style_context_add_class(gtk_widget_get_style_context(guess_button), "main-button");
    g_signal_connect(guess_button, "clicked", G_CALLBACK(check_guess), NULL);
    gtk_box_pack_start(GTK_BOX(guess_box), guess_button, FALSE, FALSE, 0);

    gtk_widget_set_sensitive(guess_entry, FALSE);
    gtk_widget_set_sensitive(guess_button, FALSE);

    g_signal_connect(guess_entry, "activate", G_CALLBACK(check_guess), NULL);

    gtk_widget_show_all(window);
}

// MAIN
int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.guessgame", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);

    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}
