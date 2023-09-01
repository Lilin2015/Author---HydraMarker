function [safty0,safty1] = safty_self(state,shape)
%% v1.0
    safty0 = ones(size(state.val));
    safty1 = ones(size(state.val));
    
    for shape_it = 1 : size(shape.val,1)
        for shape_rot = shape.rot_prop{shape_it}(:,1)'-1
            shape_cur = logical(rot90(shape.val{shape_it},shape_rot));
            if size(shape.rot_prop{shape_it},1)==4
                continue;
            end
            [shape_m,shape_n] = size(shape_cur);
            valid = conv2(double(state.val>=0),rot90(shape_cur,2),'valid');
            [valid_m,valid_n] = find(valid>0);
            % extract tags
            for it = 1 : length(valid_m)
                im = valid_m(it);
                in = valid_n(it);
                tag = state.val(im:im+shape_m-1,in:in+shape_n-1);
                
                for tag_rot = size(shape.rot_prop{shape_it},1) : size(shape.rot_prop{shape_it},1) : 3
                    tag_cur = rot90(tag,tag_rot);
                    tag_risk0 = zeros(size(tag));
                    tag_risk1 = zeros(size(tag));
                    if all(tag==tag_cur|tag==-1|tag_cur==-1,'all')
                        % is fit
                        risk = 0.5^(sum(tag==-1,'all')-1);
                        tag_risk0(tag_cur==0)=risk;
                        tag_risk1(tag_cur==1)=risk;
                    end
                        safty0(im:im+shape_m-1,in:in+shape_n-1)=safty0(im:im+shape_m-1,in:in+shape_n-1).*(1-tag_risk0);
                        safty1(im:im+shape_m-1,in:in+shape_n-1)=safty1(im:im+shape_m-1,in:in+shape_n-1).*(1-tag_risk1);
                end
            end % for tag it
        end % shape rot
    end % shape
    safty0(state.val~=-1)=1;
    safty1(state.val~=-1)=1;
% %% v2.0
%     safty0 = ones(size(state.val));
%     safty1 = ones(size(state.val));
%     
%     for shape_it = 1 : size(shape.val,1)
%         for shape_rot = shape.rot_prop{shape_it}(:,1)'-1
%             shape_cur = logical(rot90(shape.val{shape_it},shape_rot));
%             if size(shape.rot_prop{shape_it},1)==4
%                 continue;
%             end
%             [shape_m,shape_n] = size(shape_cur);
%             [rtag_m,rtag_n] = find_rtag(state,shape_cur,focus_ele);
%             
%             % extract tags
%             for it = 1 : length(rtag_m)
%                 im = rtag_m(it);
%                 in = rtag_n(it);
%                 tag = state.val(im:im+shape_m-1,in:in+shape_n-1);
%                 
%                 for tag_rot = size(shape.rot_prop{shape_it},1) : size(shape.rot_prop{shape_it},1) : 3
%                     tag_cur = rot90(tag,tag_rot);
%                     tag_risk0 = zeros(size(tag));
%                     tag_risk1 = zeros(size(tag));
%                     if all(tag==tag_cur|tag==-1|tag_cur==-1,'all')
%                         % is fit
%                         risk = 0.5^(sum(tag==-1,'all')-1);
%                         tag_risk0(tag_cur==0)=risk;
%                         tag_risk1(tag_cur==1)=risk;
%                     end
%                         safty0(im:im+shape_m-1,in:in+shape_n-1)=safty0(im:im+shape_m-1,in:in+shape_n-1).*(1-tag_risk0);
%                         safty1(im:im+shape_m-1,in:in+shape_n-1)=safty1(im:im+shape_m-1,in:in+shape_n-1).*(1-tag_risk1);
%                 end
%             end % for tag it
%         end % shape rot
%     end % shape
%     safty0(state.val~=-1)=1;
%     safty1(state.val~=-1)=1;
end

