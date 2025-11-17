#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gtk/gtk.h>

// We need to store pointers to widgets that we want to change dynamically.
// A struct is the best way to pass all this information to our callbacks.
typedef struct {
    int random_num;
    int guess_count;
    gchar player_name[50];

    // Pointers to widgets we need to access
    GtkWidget *stack;
    GtkWidget *name_entry;
    GtkWidget *name_error_label;
    GtkWidget *welcome_label;
    GtkWidget *your_guess_label; // Pointer to "Your Guess:" label
    GtkWidget *guess_entry;
    GtkWidget *feedback_label;
    GtkWidget *guess_button;
    GtkWidget *play_again_button;
    GtkWidget *guess_box; // The HBox containing guess_entry and guess_button

} GameData;

// --- Helper Functions ---

// Returns a performance string based on the guess count
const char* get_performance(int count) {
    if (count == 1) return "IMPOSSIBLE! Are you psychic?!";
    if (count >= 2 && count <= 4) return "Mastermind!";
    if (count >= 5 && count <= 7) return "Amazing Job!";
    if (count >= 8 && count <= 10) return "Well Done!";
    return "Phew! You got it just in time!";
}

// Resets the game state for a new round
void start_new_game(GameData *data) {
    // Generate a new random number
    data->random_num = (rand() % 100) + 1;
    data->guess_count = 0;

    // Update welcome message
    gchar *welcome_text = g_strdup_printf("Welcome, %s! \nI'm thinking of a secret number from 1 to 100. \nCan you read my mind?", data->player_name);
    gtk_label_set_text(GTK_LABEL(data->welcome_label), welcome_text);
    g_free(welcome_text);

    // Reset UI elements
    gtk_label_set_text(GTK_LABEL(data->feedback_label), "What's your first guess? Let's see...");
    gtk_entry_set_text(GTK_ENTRY(data->guess_entry), "");
    
    // Remove any previous color styling from feedback
    GtkStyleContext *context = gtk_widget_get_style_context(data->feedback_label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "warning");
    gtk_style_context_remove_class(context, "error");

    // Show elements for a new game
    gtk_widget_show(data->welcome_label); // Show welcome label
    gtk_widget_show(data->your_guess_label); // Show "Your Guess:" label
    gtk_widget_show(data->guess_box); // Show the guess entry and button
    gtk_widget_hide(data->play_again_button); // Hide play again button
    
    // Re-enable guessing
    gtk_widget_set_sensitive(data->guess_entry, TRUE);
    gtk_widget_set_sensitive(data->guess_button, TRUE);
    
    // Set focus to the guess entry
    gtk_widget_grab_focus(data->guess_entry);
}

// --- Callback Functions ---

// Called when the "Start Game" button is clicked
void on_start_game_clicked(GtkButton *button, gpointer user_data) {
    GameData *data = (GameData *)user_data;
    const gchar *name = gtk_entry_get_text(GTK_ENTRY(data->name_entry));

    if (strlen(name) == 0) {
        gtk_label_set_text(GTK_LABEL(data->name_error_label), "Whoops! I need a name to cheer for. Please enter one!");
        gtk_widget_show(data->name_error_label);
    } else {
        g_strlcpy(data->player_name, name, sizeof(data->player_name));
        gtk_widget_hide(data->name_error_label);
        start_new_game(data);
        gtk_stack_set_visible_child_name(GTK_STACK(data->stack), "game_screen");
    }
}

// Called when the "Guess" button is clicked or Enter is pressed in the entry
void on_guess_clicked(GtkButton *button, gpointer user_data) {
    GameData *data = (GameData *)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(data->guess_entry));
    int guessed_num = atoi(text);

    GtkStyleContext *context = gtk_widget_get_style_context(data->feedback_label);
    gtk_style_context_remove_class(context, "success");
    gtk_style_context_remove_class(context, "warning");
    gtk_style_context_remove_class(context, "error");

    // --- Validation ---
    if (guessed_num < 1 || guessed_num > 100) {
        gchar *feedback_text = g_strdup_printf("Whoa there! That's not in the rulebook.\nPlease enter a number from 1 to 100.");
        gtk_label_set_text(GTK_LABEL(data->feedback_label), feedback_text);
        g_free(feedback_text);
        gtk_style_context_add_class(context, "error");
    } else {
        // --- Guess Logic ---
        data->guess_count++;
        gchar *feedback_text;

        if (data->random_num > guessed_num) {
            feedback_text = g_strdup_printf("That's too low! Aim a little higher.");
            gtk_label_set_text(GTK_LABEL(data->feedback_label), feedback_text);
            gtk_style_context_add_class(context, "warning");
        } else if (data->random_num < guessed_num) {
            feedback_text = g_strdup_printf("Overshot it! Try a smaller number.");
            gtk_label_set_text(GTK_LABEL(data->feedback_label), feedback_text);
            gtk_style_context_add_class(context, "warning");
        } else {
            // --- Win Condition ---
            const char *performance = get_performance(data->guess_count);
            feedback_text = g_strdup_printf("<span size='large' weight='bold'>You Got It, %s!</span>\n\nYou found the secret number %d in %d guesses.\nYour performance is: <span weight='bold'>%s</span>",
                                            data->player_name, guessed_num, data->guess_count, performance);
            gtk_label_set_markup(GTK_LABEL(data->feedback_label), feedback_text);
            gtk_style_context_add_class(context, "success");

            // Hide unnecessary elements on win
            gtk_widget_set_sensitive(data->guess_entry, FALSE);
            gtk_widget_set_sensitive(data->guess_button, FALSE);
            gtk_widget_hide(data->guess_box);
            gtk_widget_hide(data->welcome_label); // Hide welcome message
            gtk_widget_hide(data->your_guess_label); // Hide "Your Guess:" label
            gtk_widget_show(data->play_again_button);
        }
        if (data->random_num != guessed_num) {
            g_free(feedback_text);
        }
    }

    if (data->random_num != guessed_num) {
         gtk_entry_set_text(GTK_ENTRY(data->guess_entry), "");
         gtk_widget_grab_focus(data->guess_entry);
    }
}

// Called when the "Play Again?" button is clicked
void on_play_again_clicked(GtkButton *button, gpointer user_data) {
    GameData *data = (GameData *)user_data;
    gtk_label_set_markup(GTK_LABEL(data->feedback_label), ""); // Clear markup
    start_new_game(data);
}

// --- UI Construction ---

// Loads custom CSS to style the application
void load_css(void) {
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);

    gtk_style_context_add_provider_for_screen(screen,
                                              GTK_STYLE_PROVIDER(provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_USER);
    
    // --- CSS UPDATED WITH YOUR NEW BUTTON STYLES ---
    const char *css =
        "/* --- Global Window --- */"
        ".window {"
        "    background-color:   #d9d9d9; /* bg-gray-100 */"
        "}"
        ""
        "/* --- Main Card --- */"
        "#main_card {"
        "    background-color: #ffffff; /* bg-white */"
        "    border-radius: 10px;"
        "    padding: 24px;"
        "    border: 1px solid #e5e7eb;"
        "}"
        ""
        "/* --- Title --- */"
        "#title_label {"
        "    font-size: 22pt;"
        "    font-weight: bold;"
        "    margin-bottom: 16px;"
        "    color: #1f2937; /* Darker text */"
        "}"
        ""
        "/* --- Standard Labels (e.g., 'Your Name:') --- */"
        "label {"
        "    font-size: 11pt;"
        "    color: #000000; /* Gray text */"
        "    margin-top: 8px;"
        "}"
        ""
        "/* --- Entry Fields --- */"
        "entry {"
        "    font-size: 11pt;"
        "    padding: 8px 12px;"
        "    border: 1px solid #d1d5db; /* border-gray-300 */"
        "    border-radius: 8px;"
        "}"
        ""
        "entry:focus {"
        "    border: 2px solid #3b82f6; /* focus:ring-blue-500 */"
        "    padding: 7px 11px; /* Adjust padding to account for thicker border */"
        "}"
        ""
        "entry:hover {"
        "    border-color: #a0a0a0; /* Slightly darker border on hover */"
        "}"
        ""
        "entry:disabled {"
        "    background-color: #f3f4f6; /* bg-gray-100 */"
        "    color: #9ca3af; /* text-gray-400 */"
        "    border-color: #e5e7eb;"
        "}"
        ""
        "/* --- Buttons (YOUR NEW STYLES) --- */"
        "button {"
        "    font-size: 12pt;"
        "    font-weight: bold;"
        "    border-radius: 8px;"
        "    padding: 10px 16px;"
        "    color: #ffffff !important;" /* Default white text */
        "    border: none;"
        "    margin-top: 10px;"
        "    transition: background-color 0.2s ease-in-out;"
        "    background-image: none; /* Remove default theme gradient */"
        "}"
        ""
        "button:hover { "
        "    background-color: #60a5fa; /* Light blue on hover */"
        "}"
        ""
        "button:active {"
        "    background-color: #00277c; /* Darker blue when pressed */"
        "}"
        ""
        "button:disabled {"
        "    background-color: #0b61e1; /* bg-gray-300 */"
        "    color: #ffffff; /* White text for disabled buttons too */"
        "    background-image: none; /* No hover/active effect when disabled */"
        "}"
        ""
        "#start_game_button, #guess_button, #play_again_button {"
        "    background-color: #105cd7; /* Main blue color */"
        "    background-image: none; /* Ensure no gradient */"
        "    color: #ffffff; /* <--- ADDED THIS FOR EXPLICIT WHITE TEXT */"
        "}"
        ""
        "/* --- NEW RULE: Specific hover for buttons by ID --- */"
        "#start_game_button:hover, #guess_button:hover, #play_again_button:hover {"
        "    background-color: #60a5fa; /* Light blue on hover */"
        "    background-image: none; /* Ensure no gradient */"
        "}"
        ""
        "/* --- Feedback/Error Labels --- */"
        "#name_error_label {"
        "    color: #ef4444; /* text-red-500 */"
        "    font-size: 12pt;"
        "    margin-top: 8px;"
        "}"
        ""
        "/* Default feedback label ('It's your turn...') */"
        "#feedback_label {"
        "    font-size: 14pt;"
        "    font-weight: 500;"
        "    color: #4a4a4a; /* Dark gray text */"
        "    min-height: 70px;"
        "    padding: 16px 0; /* Vertical padding only */"
        "    margin-top: 10px;"
        "}"
        ""
        "/* Feedback label when showing a result (warning, error, success) */"
        "#feedback_label.warning, #feedback_label.error, #feedback_label.success {"
        "    background-color:       #b3b3b3; /* bg-gray-50 */"
        "    border-radius: 8px;"
        "    padding: 16px;" /* Full padding */
        "}"
        ""
        "/* --- Feedback Colors --- */"
        "#feedback_label.warning { color: #ca8a04; } /* text-yellow-600 */"
        "#feedback_label.error { color: #dc2626; } /* text-red-600 */"
        "#feedback_label.success { color: #16a34a; } /* text-green-600 */"
        ""
        "#welcome_label {"
        "    font-size: 14pt;"
        "    margin-bottom: 12px;"
        "    color: #374151;"
        "}";

    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    g_object_unref(provider);
}

// Builds the "Name Input" screen
GtkWidget* create_name_screen(GameData *data) {
    GtkWidget *vbox, *label, *button;

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    label = gtk_label_new("What should I call you, challenger?");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    data->name_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->name_entry), "Enter your legendary name...");
    gtk_box_pack_start(GTK_BOX(vbox), data->name_entry, FALSE, FALSE, 0);

    button = gtk_button_new_with_label("Start Game");
    gtk_widget_set_name(button, "start_game_button"); // For CSS
    g_signal_connect(button, "clicked", G_CALLBACK(on_start_game_clicked), data);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);

    g_signal_connect(data->name_entry, "activate", G_CALLBACK(on_start_game_clicked), data);

    data->name_error_label = gtk_label_new("");
    gtk_widget_set_name(data->name_error_label, "name_error_label"); // For CSS
    gtk_widget_set_halign(data->name_error_label, GTK_ALIGN_CENTER);
    gtk_widget_set_no_show_all(data->name_error_label, TRUE); // Hide initially
    gtk_box_pack_start(GTK_BOX(vbox), data->name_error_label, FALSE, FALSE, 0);

    return vbox;
}

// Builds the "Main Game" screen
GtkWidget* create_game_screen(GameData *data) {
    GtkWidget *vbox, *hbox; 

    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

    data->welcome_label = gtk_label_new("");
    gtk_widget_set_name(data->welcome_label, "welcome_label"); // For CSS
    gtk_label_set_justify(GTK_LABEL(data->welcome_label), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(data->welcome_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), data->welcome_label, FALSE, FALSE, 0);

    data->your_guess_label = gtk_label_new("Your Guess:");
    gtk_widget_set_halign(data->your_guess_label, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(vbox), data->your_guess_label, FALSE, FALSE, 0);

    // --- Guess Input Box (HBox) ---
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    data->guess_box = hbox; // Save pointer to show/hide

    data->guess_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(data->guess_entry), "1-100");
    gtk_box_pack_start(GTK_BOX(hbox), data->guess_entry, TRUE, TRUE, 0);

    data->guess_button = gtk_button_new_with_label("Guess");
    gtk_widget_set_name(data->guess_button, "guess_button"); // For CSS
    g_signal_connect(data->guess_button, "clicked", G_CALLBACK(on_guess_clicked), data);
    gtk_box_pack_start(GTK_BOX(hbox), data->guess_button, FALSE, FALSE, 0);

    g_signal_connect(data->guess_entry, "activate", G_CALLBACK(on_guess_clicked), data);

    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    // --- Feedback Area ---
    data->feedback_label = gtk_label_new("What's your first guess? Let's see...");
    gtk_widget_set_name(data->feedback_label, "feedback_label"); // For CSS
    gtk_label_set_justify(GTK_LABEL(data->feedback_label), GTK_JUSTIFY_CENTER);
    gtk_label_set_line_wrap(GTK_LABEL(data->feedback_label), TRUE);
    gtk_box_pack_start(GTK_BOX(vbox), data->feedback_label, FALSE, FALSE, 0);

    // --- Play Again Button ---
    data->play_again_button = gtk_button_new_with_label("Play Again?");
    gtk_widget_set_name(data->play_again_button, "play_again_button"); // For CSS
    gtk_widget_set_no_show_all(data->play_again_button, TRUE); // Hide initially
    g_signal_connect(data->play_again_button, "clicked", G_CALLBACK(on_play_again_clicked), data);
    gtk_box_pack_start(GTK_BOX(vbox), data->play_again_button, FALSE, FALSE, 0);

    return vbox;
}

// Main function to build the entire UI
void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window, *main_card, *title_label;
    GtkWidget *name_screen, *game_screen;
    
    // Allocate memory for our GameData struct
    GameData *data = g_slice_new(GameData);

    // Seed the random number generator
    srand(time(0));

    // Create the main window
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Number Guessing Game");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_widget_set_name(window, "window"); // For CSS background

    // Create the central white card (a GtkBox)
    main_card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(main_card, "main_card"); // For CSS
    gtk_widget_set_halign(main_card, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(main_card, GTK_ALIGN_CENTER);
    gtk_widget_set_size_request(main_card, 350, -1); // Set a min-width
    gtk_container_add(GTK_CONTAINER(window), main_card);

    // Add Title
    title_label = gtk_label_new("Number Guessing Game");
    gtk_widget_set_name(title_label, "title_label"); // For CSS
    gtk_box_pack_start(GTK_BOX(main_card), title_label, FALSE, FALSE, 0);

    // Create the Stack to switch between screens
    data->stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(data->stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    
    // Create the two screens
    name_screen = create_name_screen(data);
    game_screen = create_game_screen(data);

    // Add screens to the stack
    gtk_stack_add_named(GTK_STACK(data->stack), name_screen, "name_screen");
    gtk_stack_add_named(GTK_STACK(data->stack), game_screen, "game_screen");

    // Add the stack to the main card
    gtk_box_pack_start(GTK_BOX(main_card), data->stack, TRUE, TRUE, 0);

    // Load the custom CSS
    load_css();

    // Show all widgets
    gtk_widget_show_all(window);
    
    // Hide widgets that should not be visible at start
    gtk_widget_hide(data->name_error_label);
    gtk_widget_hide(data->play_again_button);
    
    // Set focus to the name entry on start
    gtk_widget_grab_focus(data->name_entry);
}

// --- Main Program Entry ---
int main(int argc, char **argv) {
    GtkApplication *app;
    int status;

    app = gtk_application_new("com.example.gtk.numberguess", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}