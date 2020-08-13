#include "GL/glut.h"
#include "othello.hpp"
#include "othello_algorithm.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <stdexcept>
#include <thread>
enum class game_turn {
    player,
    enemy,
};
static game_turn turn = game_turn::player;
static othello::stone stone_player = othello::stone::black();
static othello::board b;
static unsigned int level = 0;
static constexpr double r = 40;
static double theta = 3 * M_PI / 2;
static double phi = 1.2 * M_PI / 2;
static double pot_angle[8][8];
static std::random_device seed_gen;
static std::default_random_engine engine(seed_gen());
static std::uniform_real_distribution<> dist(0, 360);
static int mouse_x = 0, mouse_y = 0;
static bool mouse_rb_on = false;
void play_enemy_async(int delay_milliseconds = 0)
{
    std::thread p(
        [delay_milliseconds] {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_milliseconds));
            turn = game_turn::enemy;
            try {
                auto c = play_best_hand(b, stone_player.get_reversed(), level);
                pot_angle[c.y][c.x] = dist(engine);
            } catch (const othello::board::operation_error &) {
                // cant put anywhere
            }
            turn = game_turn::player;
        });
    p.detach();
};
void resize(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, static_cast<float>(w) / h, 10.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}
void display()
{
    constexpr float box_size = 2;
    constexpr GLfloat board_ambi[] = {0.0, 0.05, 0.0, 1.0};
    constexpr GLfloat board_diff[] = {0.4, 0.5, 0.4, 1.0};
    constexpr GLfloat board_spec[] = {0.04, 0.7, 0.04, 1.0};
    constexpr GLfloat board_shine[] = {10.0};
    constexpr GLfloat line_ambi[] = {0.0, 0.0, 0.0, 1.0};
    constexpr GLfloat line_diff[] = {0.0, 0.0, 0.0, 1.0};
    constexpr GLfloat line_spec[] = {0.0, 0.0, 0.0, 1.0};
    constexpr GLfloat line_shine[] = {0.0};
    constexpr GLfloat black_pot_ambi[] = {0.05375, 0.05, 0.06625, 1.0};
    constexpr GLfloat black_pot_diff[] = {0.18275, 0.17, 0.22525, 1.0};
    constexpr GLfloat black_pot_spec[] = {0.332741, 0.328634, 0.346435, 1.0};
    constexpr GLfloat black_pot_shine[] = {38.4};
    constexpr GLfloat white_pot_ambi[] = {0.25, 0.20725, 0.20725, 1.0};
    constexpr GLfloat white_pot_diff[] = {1, 0.829, 0.829, 1.0};
    constexpr GLfloat white_pot_spec[] = {0.296648, 0.296648, 0.296648, 1.0};
    constexpr GLfloat white_pot_shine[] = {10.24};
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(
        r * std::sin(phi) * std::cos(theta),
        r * std::cos(phi),
        r * std::sin(phi) * std::sin(theta),
        0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_STENCIL_TEST);

    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, board_ambi);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, board_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, board_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, board_shine);
    glTranslatef(0, -3.5 * box_size, 0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    for (int y = 0; y < 8; ++y) {
        glTranslatef(-3.5 * box_size, 0, 0);
        for (int x = 0; x < 8; ++x) {
            glStencilFunc(GL_ALWAYS, y * 8 + x + 1, 0xff);
            glutSolidCube(box_size);
            glTranslatef(box_size, 0, 0);
        }
        glTranslatef(-4.5 * box_size, 0, 0);
        glTranslatef(0, box_size, 0);
    }
    glPopMatrix();

    glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, line_ambi);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_diff);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, line_spec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, line_shine);
    glTranslatef(0, -3.5 * box_size, -0.01);
    for (int y = 0; y < 8; ++y) {
        glTranslatef(-3.5 * box_size, 0, 0);
        for (int x = 0; x < 8; ++x) {
            glBegin(GL_LINE_LOOP);
            glVertex3f(-box_size / 2, -box_size / 2, -box_size / 2);
            glVertex3f(-box_size / 2, +box_size / 2, -box_size / 2);
            glVertex3f(+box_size / 2, +box_size / 2, -box_size / 2);
            glVertex3f(+box_size / 2, -box_size / 2, -box_size / 2);
            glEnd();
            glTranslatef(box_size, 0, 0);
        }
        glTranslatef(-4.5 * box_size, 0, 0);
        glTranslatef(0, box_size, 0);
    }
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, -3.5 * box_size, -box_size);
    for (int y = 0; y < 8; ++y) {
        glTranslatef(-3.5 * box_size, 0, 0);
        for (int x = 0; x < 8; ++x) {
            if (auto s = b(x, y)) {
                if (s == othello::stone::black()) {
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black_pot_ambi);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, black_pot_diff);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, black_pot_spec);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, black_pot_shine);
                } else {
                    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white_pot_ambi);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white_pot_diff);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_pot_spec);
                    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, white_pot_shine);
                }
                glPushMatrix();
                glRotatef(-90, 1, 0, 0);
                glRotatef(pot_angle[y][x], 0, 1, 0);
                glutSolidTeapot(0.5);
                glPopMatrix();
            }
            glTranslatef(box_size, 0, 0);
        }
        glTranslatef(-4.5 * box_size, 0, 0);
        glTranslatef(0, box_size, 0);
    }
    glPopMatrix();
    glutSwapBuffers();
}
void mouse_moves(int x, int y)
{
    mouse_x = x;
    mouse_y = y;
    glutPostRedisplay();
}
void mouse_click(int button, int state, int x, int y)
{
    if (button == GLUT_RIGHT_BUTTON) {
        mouse_rb_on = state == GLUT_DOWN;
    }
    if (turn == game_turn::enemy) return;
    int index;
    glReadPixels(mouse_x, glutGet(GLUT_WINDOW_HEIGHT) - mouse_y, 1, 1, GL_STENCIL_INDEX, GL_INT, &index);
    if (index == 0) return;
    if (auto puttable = b.get_puttable_places(stone_player); !puttable.empty()) {
        othello::board::coordinate c{(index - 1) % 8, (index - 1) / 8};
        if (std::count(puttable.begin(), puttable.end(), c)) {
            b.put(c, stone_player);
            pot_angle[c.y][c.x] = dist(engine);
            play_enemy_async(50);
        } else {
            // cant put here
        }
    } else {
        // cant put anywhere
        play_enemy_async(50);
    }
}
void drag_motion(int x, int y)
{
    constexpr float magnitude = 0.01;
    if (mouse_rb_on) {
        theta += (x - mouse_x) * magnitude;
        phi = std::clamp(phi + (mouse_y - y) * magnitude, 0.001, M_PI);
        mouse_x = x;
        mouse_y = y;
    }
    glutPostRedisplay();
}
void lightInit(void)
{
    constexpr GLfloat light0pos[] = {0.0, 0.0, -10.0, 1.0};
    constexpr GLfloat light0ambi[] = {0.2, 0.2, 0.2, 1.0};
    constexpr GLfloat light0diff[] = {0.8, 0.8, 0.8, 1.0};
    constexpr GLfloat light0spec[] = {0.5, 0.5, 0.5, 1.0};
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);
    glLightfv(GL_LIGHT0, GL_POSITION, light0pos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light0ambi);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0spec);
    glShadeModel(GL_SMOOTH);
}
int main(int argc, char *argv[])
{
    try {
        for (int i = 1; i < argc; ++i) {
            if (std::sscanf(argv[i], "-level=%u", &level)) {
            } else if (!std::strcmp(argv[i], "-second")) {
                turn = game_turn::enemy;
                stone_player = othello::stone::white();
            } else {
                throw std::invalid_argument("invalid arguments.");
            }
        }
        pot_angle[3][3] = dist(engine);
        pot_angle[3][4] = dist(engine);
        pot_angle[4][3] = dist(engine);
        pot_angle[4][4] = dist(engine);
        if (turn == game_turn::enemy) {
            play_enemy_async(50);
        }
        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
        glutInitWindowSize(640, 480);
        glutCreateWindow("window");
        glutDisplayFunc(display);
        glutReshapeFunc(resize);
        glutPassiveMotionFunc(mouse_moves);
        glutMotionFunc(drag_motion);
        glutMouseFunc(mouse_click);
        lightInit();
        glutMainLoop();
    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
