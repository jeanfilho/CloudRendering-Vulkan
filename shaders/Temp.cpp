class AnisotropicPhaseBSDF : public BSDF
{
public:
    AnisotropicPhaseBSDF(float g, ShadingContext &ctx) : BSDF(ctx), m_g(g)
    {
        if (m_g < -1.0f)
            m_g = -1.0f;
        if (m_g > 1.0f)
            m_g = 1.0f;
        m_isotropic = (fabsf(m_g) < 0.0001f);
        
        // Precompute terms used for the pdf/cdf
        if (!m_isotropic)
        {
            m_one_plus_g2 = 1.0f + m_g * m_g;
            m_one_minus_g2 = 1.0f - m_g * m_g;
            m_one_over_2g = 0.5f / m_g;
        }
    }
    virtual void EvaluateSample(RendererServices &rs, const Vector &sampleDirection, Color &L, float &pdf)
    {
        if (m_isotropic)
            pdf = 0.25 / M_PI;
        else
        {
            float cosTheta = Dot(-m_ctx.GetV(), sampleDirection);
            pdf = calcpdf(cosTheta);
        }
        L = Color(pdf);
    }
    virtual void GenerateSample(RendererServices &rs, Vector &sampleDirection, Color &L,
                                float &pdf)
    {
        if (m_isotropic)
        {
            // same as IsotropicPhaseBSDF::GenerateSample
        }
        else
        {
            float phi = rs.GenerateRandomNumber() * 2 * M_PI;
            float cosTheta = invertcdf(rs.GenerateRandomNumber());
            float sinTheta = sqrtf(1.0f - cosTheta * cosTheta); // actually square of
            sinTheta
                Vector in = -m_ctx.GetV();
            Vector t0, t1;
            in.CreateOrthonormalBasis(t0, t1);
            sampleDirection = sinTheta * sinf(phi) * t0 + sinTheta * cosf(phi) * t1 +
                              cosTheta * in;
            pdf = calcpdf(cosTheta);
        }
        L = Color(pdf);
    }

private:
    float calcpdf(float costheta)
    {
        return 0.25 * m_one_minus_g2 / (M_PI * powf(m_one_plus_g2 - 2.0f * m_g * costheta, 1.5f));
    }
    // Assumes non-isotropic due to division by m_g
    float invertcdf(float xi)
    {
        float t = (m_one_minus_g2) / (1.0f - m_g + 2.0f * m_g * xi);
        return m_one_over_2g * (m_one_plus_g2 - t * t);
    }
    float m_g, m_one_plus_g2, m_one_minus_g2, m_one_over_2g;
    bool m_isotropic;
};
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
class MultiScatterHeterogeneousVolume : public Volume
{
public:
    MultiScatterHeterogeneousVolume(
        int scatteringAlbedoProperty,
        Color &maxExtinction,
        int extinctionProperty,
        ShadingContext &ctx)
        : Volume(ctx), m_scatteringAlbedoProperty(scatteringAlbedoProperty),
          m_maxExtinction(maxExtinction.ChannelAvg()),
          m_extinctionProperty(extinctionProperty) {}
    virtual bool Integrate(
        RendererServices &rs,
        const Ray &wi,
        Color &L,
        Color &transmittance,
        Color &weight,
        Point &P,
        Ray &wo,
        Geometry &g)
    {
        Point P0 = m_ctx.GetP();
        if (!rs.GetNearestHit(Ray(P0, wi.dir), P, g))
            return false;
        float distance = Vector(P - P0).Length();
        bool terminated = false;
        float t = 0;
        do
        {
            float zeta = rs.GenerateRandomNumber();
            t = t - log(1 - zeta) / m_maxExtinction;
            if (t > distance)
            {
                break; // Did not terminate in the volume
            }
            // Update the shading context
            Point P = P0 + t * wi.dir;
            m_ctx.SetP(P);
            m_ctx.RecomputeInputs();
            // Recompute the local extinction after updating the shading context
            Color extinction = m_ctx.GetColorProperty(m_extinctionProperty);
            float xi = rs.GenerateRandomNumber();
            if (xi < (extinction.ChannelAvg() / m_maxExtinction))
                terminated = true;
        } while (!terminated);

        if (terminated)
        {
            P = m_ctx.GetP() + t * wi.dir;
            g = m_ctx.GetGeometry();
            Color transmittance = Color(1.0f);
            Color extinction = m_ctx.GetColorProperty(m_extinctionProperty);
            Color pdf = extinction * transmittance;
            Color scatteringAlbedo = m_ctx.GetColorProperty(m_scatteringAlbedoProperty);
            // Note that pdf already has extinction in it, so we should avoid the
            // multiply and divide; it is shown here for clarity
            weight = scatteringAlbedo * extinction / pdf;
        }
        else
        {
            Color transmittance = Color(1.0f);
            Color pdf = transmittance;
            weight = 1.0 / pdf;
        }
        L = Color(0.0); // No estimate of radiance
        wo = Ray(P, wi.dir);
        return true;
    }

    virtual Color Transmittance(RendererServices &rs, const Point &P0, const Point &P1)
    {
        float distance = Vector(P0 - P1).Length();
        return Color(exp(m_extinction.r * -distance), exp(m_extinction.g * -distance),
                     exp(m_extinction.b * -distance));
    }

protected:
    const int m_scatteringAlbedoProperty;
    const float m_maxExtinction;
    const int m_extinctionProperty;
};
//-------------------------------------------------------------------------------
//-------------------------------------------------------------------------------
// Light Integrator for a path - fwd path tracer
void LightIntegrator()
{
    Color L = Color(0.0);
    Color throughput = Color(1.0);
    Ray ray = pickCameraDirection();
    if (rs.GetNearestHit(ray, P, g))
        continue;
    int j = 0;
    while (j < maxPathLength)
    {
        ShadingContext *ctx = rs.CreateShadingContext(P, ray, g);
        Material *m = g.GetMaterial();
        BSDF *bsdf = m->CreateBSDF(*ctx);

        // Perform direct lighting on the surface
        L += throughput * directLighting();

        // Compute direction of indirect ray
        float pdf;
        Color Ls;
        Vector sampleDirection;
        bsdf->GenerateSample(rs, sampleDirection, Ls, pdf);
        throughput *= (Ls / pdf);
        Ray nextRay(ray);
        nextRay.org = P;
        nextRay.dir = sampleDirection;

        Volume *volume = 0;
        if (m->HasVolume())
        {
            // Did we go transmit through the surface? V is the
            // direction away from the point P on the surface.
            float VdotN = ctx->GetV().Dot(ctx->GetN());
            float dirDotN = sampleDirection.Dot(ctx->GetN());
            bool transmit = (VdotN < 0.0) != (dirDotN < 0.0);
            if (transmit)
            {
                // We transmitted through the surface. Check dot
                // product between the sample direction and the
                // surface normal N to see whether we entered or
                // exited the volume media
                bool entered = dirDotN < 0.0f;
                if (entered)
                {
                    nextRay.EnterMaterial(m);
                }
                else
                {
                    nextRay.ExitMaterial(m);
                }
            }
            volume = nextRay.GetVolume(*ctx);
        }

        if (volume)
        {
            Color Lv;
            Color transmittance;
            float weight;

            if (!volume->Integrate(rs, nextRay, Lv, transmittance, weight, P, nextRay, g))
                break;
            L += weight * throughput * Lv;
            throughput *= transmittance;
        }
        else
        {
            if (!rs.GetNearestHit(nextRay, P, g))
                break;
        }

        ray = nextRay;
        j++;
    }
}