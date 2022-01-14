#define OMM_ALL_HEADERS
#include "data/omm/omm_includes.h"
#undef OMM_ALL_HEADERS

void vec3f_norm(Vec3f v) {
    f32 dv = sqrtf(omm_sqr_f(v[0]) + omm_sqr_f(v[1]) + omm_sqr_f(v[2]));
    if (dv != 0) {
        v[0] /= dv;
        v[1] /= dv;
        v[2] /= dv;
    }
}

void vec3f_set_mag(Vec3f v, f32 mag) {
    vec3f_norm(v);
    vec3f_mul(v, mag);
}

f32 vec3f_dist(Vec3f v1, Vec3f v2) {
    return sqrtf(omm_sqr_f(v2[0] - v1[0]) + omm_sqr_f(v2[1] - v1[1]) + omm_sqr_f(v2[2] - v1[2]));
}

f32 vec3f_hdist(Vec3f v1, Vec3f v2) {
    return sqrtf(omm_sqr_f(v2[0] - v1[0]) + omm_sqr_f(v2[2] - v1[2]));
}

void vec3f_mult(Vec3f dest, Vec3f v1, Vec3f v2) {
    dest[0] = v1[0] * v2[0];
    dest[1] = v1[1] * v2[1];
    dest[2] = v1[2] * v2[2];
}

void vec3f_to_polar_coords(Vec3f v, f32 *dist, s16 *pitch, s16 *yaw) {
    if (v[0] == 0.f && v[1] == 0.f && v[2] == 0.f) {
        if (dist) {
            *dist = 0.f;
        }
        if (pitch) {
            *pitch = 0;
        }
        if (yaw) {
            *yaw = 0;
        }
    } else {
        if (dist) {
            *dist = sqrtf(omm_sqr_f(v[0]) + omm_sqr_f(v[1]) + omm_sqr_f(v[2]));
        }
        if (pitch) {
            *pitch = atan2s(sqrtf(omm_sqr_f(v[0]) + omm_sqr_f(v[2])), v[1]);
        }
        if (yaw) {
            *yaw = atan2s(v[2], v[0]);
        }
    }
}

void vec3f_get_nullspace(Vec3f destAxisN, Vec3f destAxisE1, Vec3f destAxisE2, Vec3f n) {
    vec3f_set(destAxisN, n[0], n[1], n[2]);
    if (destAxisN[0] == 0.f && destAxisN[1] == 0.f) {
        vec3f_set(destAxisE1, 1.f, 0.f, 0.f);    
        vec3f_set(destAxisE2, 0.f, 1.f, 0.f);
    } else {
        vec3f_set(destAxisE1, -destAxisN[1], destAxisN[0], 0.f);
        vec3f_set(destAxisE2, -destAxisN[0] * destAxisN[2], -destAxisN[1] * destAxisN[2], omm_sqr_f(destAxisN[0]) + omm_sqr_f(destAxisN[1]));
    }
    vec3f_norm(destAxisN);
    vec3f_norm(destAxisE1);
    vec3f_norm(destAxisE2);
}

void vec3f_project_point(Vec3f dest, Vec3f v, Vec3f o, Vec3f n) {
    f32 dot = (v[0] - o[0]) * n[0] + (v[1] - o[1]) * n[1] + (v[2] - o[2]) * n[2];
    dest[0] = v[0] - dot * n[0];
    dest[1] = v[1] - dot * n[1];
    dest[2] = v[2] - dot * n[2];
}

void vec3f_project_vector(Vec3f dest, Vec3f v, Vec3f n) {
    f32 dot = v[0] * n[0] + v[1] * n[1] + v[2] * n[2];
    dest[0] = v[0] - dot * n[0];
    dest[1] = v[1] - dot * n[1];
    dest[2] = v[2] - dot * n[2];
}

void vec3f_to_2d_plane(Vec2f dest2d, f32 *dist2d, Vec3f src3d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2) {
    Vec3f op = { src3d[0] - o[0], src3d[1] - o[1], src3d[2] - o[2] };
    dest2d[0] = vec3f_dot(e1, op);
    dest2d[1] = vec3f_dot(e2, op);
    *dist2d = vec3f_dot(n, op);
}

void vec2f_and_dist_to_3d(Vec3f dest3d, Vec2f src2d, f32 dist2d, Vec3f o, Vec3f n, Vec3f e1, Vec3f e2) {
    dest3d[0] = o[0] + src2d[0] * e1[0] + src2d[1] * e2[0] + dist2d * n[0];
    dest3d[1] = o[1] + src2d[0] * e1[1] + src2d[1] * e2[1] + dist2d * n[1];
    dest3d[2] = o[2] + src2d[0] * e1[2] + src2d[1] * e2[2] + dist2d * n[2];
}

void vec2f_to_3d_plane(Vec3f dest, Vec2f src, Vec3f o, Vec3f e1, Vec3f e1Scale, Vec3f e2, Vec3f e2Scale) {
    dest[0] = o[0] + src[0] * e1[0] * e1Scale[0] + src[1] * e2[0] * e2Scale[0];
    dest[1] = o[1] + src[0] * e1[1] * e1Scale[1] + src[1] * e2[1] * e2Scale[1];
    dest[2] = o[2] + src[0] * e1[2] * e1Scale[2] + src[1] * e2[2] * e2Scale[2];
}

void vec3f_rotate_zxy(Vec3f dest, Vec3f v, s16 pitch, s16 yaw, s16 roll) {
    Vec3f t = { 0, 0, 0 };
    Vec3s r = { pitch, yaw, roll };
    Mat4 mtx;
    mtxf_rotate_zxy_and_translate(mtx, t, r);
    f32 x = v[0];
    f32 y = v[1];
    f32 z = v[2];
    dest[0] = x * mtx[0][0] + y * mtx[1][0] + z * mtx[2][0] + mtx[3][0];
    dest[1] = x * mtx[0][1] + y * mtx[1][1] + z * mtx[2][1] + mtx[3][1];
    dest[2] = x * mtx[0][2] + y * mtx[1][2] + z * mtx[2][2] + mtx[3][2];
}

// Rodrigues' formula
// dest = v * cos(r) + (n x v) * sin(r) + n * (n . v) * (1 - cos(r))
void vec3f_rotate_around_n(Vec3f dest, Vec3f v, Vec3f n, s16 r) {
    Vec3f kCrossv;
    vec3f_cross(kCrossv, n, v);
    f32 kDotv = vec3f_dot(n, v);
    f32 cosr = coss(r);
    f32 sinr = sins(r);
    dest[0] = v[0] * cosr + kCrossv[0] * sinr + n[0] * kDotv * (1.f - cosr);
    dest[1] = v[1] * cosr + kCrossv[1] * sinr + n[1] * kDotv * (1.f - cosr);
    dest[2] = v[2] * cosr + kCrossv[2] * sinr + n[2] * kDotv * (1.f - cosr);
}

// Scale, rotate, translate
void vec3f_transform(Vec3f dest, Vec3f v, Vec3f translation, Vec3s rotation, Vec3f scale) {
    vec3f_copy(dest, v);
    vec3f_mult(dest, dest, scale);
    vec3f_rotate_zxy(dest, dest, rotation[0], rotation[1], rotation[2]);
    vec3f_sum(dest, dest, translation);
}

// Lagrange's polynoms
//                t - t1      t - t2           t - t0      t - t2           t - t0      t - t1
// L3(t) = p0 * --------- * --------- + p1 * --------- * --------- + p2 * --------- * ---------
//               t0 - t1     t0 - t2          t1 - t0     t1 - t2          t2 - t0     t2 - t1
void vec3f_interpolate(Vec3f dest, f32 t, Vec3f p0, f32 t0, Vec3f p1, f32 t1, Vec3f p2, f32 t2) {
    f32 m0 = ((t - t1) / (t0 - t1)) * ((t - t2) / (t0 - t2));
    f32 m1 = ((t - t0) / (t1 - t0)) * ((t - t2) / (t1 - t2));
    f32 m2 = ((t - t0) / (t2 - t0)) * ((t - t1) / (t2 - t1));
    dest[0] = p0[0] * m0 + p1[0] * m1 + p2[0] * m2;
    dest[1] = p0[1] * m0 + p1[1] * m1 + p2[1] * m2;
    dest[2] = p0[2] * m0 + p1[2] * m1 + p2[2] * m2;
}

bool vec3f_is_inside_cylinder(Vec3f v, Vec3f pos, f32 radius, f32 height, f32 downOffset) {
    f32 hDist = vec3f_hdist(v, pos);
    f32 vDist = v[1] - pos[1] + downOffset;
    return hDist < radius && vDist >= 0.f && vDist < height;
}

void vec2f_get_projected_point_on_line(Vec2f dest, Vec2f p, Vec2f a, Vec2f b) {
    Vec2f vab = { b[0] - a[0], b[1] - a[1] };
    Vec2f vap = { p[0] - a[0], p[1] - a[1] };
    f32 dpbap = (vab[0] * vap[0]) + (vab[1] * vap[1]);
    f32 vabn = sqrtf(omm_sqr_f(vab[0]) + omm_sqr_f(vab[1]));
    f32 vapn = sqrtf(omm_sqr_f(vap[0]) + omm_sqr_f(vap[1]));
    f32 cospab = dpbap / (vabn * vapn);
    f32 vaqn = cospab * vapn;
    dest[0] = a[0] + (vaqn * vab[0]) / vabn;
    dest[1] = a[1] + (vaqn * vab[1]) / vabn;
}
