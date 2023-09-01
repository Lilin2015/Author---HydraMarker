function result = generate_MF(state,shape,method,max_time,max_iter)
    unum = sum(state==-1,'all');
    if ~exist('max_time','var')
        max_time = 400;
    end
    if ~exist('max_iter','var')
        max_iter = 20*unum;
    end
    if strcmp(method,'bWFC-sf')
        Gen = CHydraMarker_gen();
        Gen.assign(Dfield(state),Dshape(shape),max_time,max_iter);
        Gen.generate('s-first');
        result.state = Gen.state;
        result.record = Gen.process_record;
    elseif strcmp(method,'bWFC-pf')
        Gen = CHydraMarker_gen();
        Gen.assign(Dfield(state),Dshape(shape),max_time,max_iter);
        Gen.generate('p-first');
        result.state = Gen.state;
        result.record = Gen.process_record;
    elseif strcmp(method,'bWFC-l')    
        Gen = CHydraMarker_gen();
        Gen.assign(Dfield(state),Dshape(shape),max_time,max_iter);
        Gen.generate('l-only');
        result.state = Gen.state;
        result.record = Gen.process_record;
    elseif strcmp(method,'bWFC-c')
        Gen = CHydraMarker_gen();
        Gen.assign(Dfield(state),Dshape(shape),max_time,max_iter);
        Gen.generate('c-only');
        result.state = Gen.state;
        result.record = Gen.process_record;
    elseif strcmp(method,'fast-bWFC')
        [state,process_record] = method_bWFC_fast(Dfield(state),Dshape(shape),max_time,max_iter);
        result.state = state;
        result.record = process_record;
    elseif strcmp(method,'monkey-filling')
        [state,process_record] = monkey_filling(Dfield(state),Dshape(shape),max_time,max_iter);
        result.state = state;
        result.record = process_record;
    elseif strcmp(method,'linear-filling')
        [state,process_record] = linear_filling(Dfield(state),Dshape(shape),max_time,max_iter);
        result.state = state;
        result.record = process_record;
    elseif strcmp(method,'DOF-filling')
        [state,process_record] = DOF_filling(Dfield(state),Dshape(shape),max_time,max_iter);
        result.state = state;
        result.record = process_record;
    elseif strcmp(method,'Gao')
        loc_size = max(size(shape{1,1}));
        gol_size = max(size(state))-loc_size+1;
        [state,process_record] = Gao_filling(gol_size,loc_size,max_time,max_iter);
        result.state = state;
        result.record = process_record;
    else
        fprintf('\nno such method!\n');
        result = [];
    end
end

